// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/06/11.

#include "polar/global/platform/Unix.h"
#include "polar/global/Config.h"
#include "polar/global/ManagedStatic.h"
#include "polar/basic/adt/StlExtras.h"
#include "polar/utils/FileSystem.h"
#include "polar/utils/FileUtils.h"
#include "polar/utils/Format.h"
#include "polar/utils/MemoryBuffer.h"
#include "polar/utils/Program.h"
#include "polar/utils/RawOutStream.h"
#include "polar/utils/Allocator.h"
#include "polar/utils/Signals.h"
#include "polar/utils/StringSaver.h"

#include <algorithm>
#include <string>
#include <atomic>
#include <mutex>

#ifdef HAVE_BACKTRACE
# include <execinfo.h>         // For backtrace().
#endif
#if HAVE_SIGNAL_H
#include <signal.h>
#endif
#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#if HAVE_DLFCN_H
#include <dlfcn.h>
#endif
#if HAVE_MACH_MACH_H
#include <mach/mach.h>
#endif
#if HAVE_LINK_H
#include <link.h>
#endif
#ifdef HAVE__UNWIND_BACKTRACE
// FIXME: We should be able to use <unwind.h> for any target that has an
// _Unwind_Backtrace function, but on FreeBSD the configure test passes
// despite the function not existing, and on Android, <unwind.h> conflicts
// with <link.h>.
#ifdef __GLIBC__
#include <unwind.h>
#else
#undef HAVE__UNWIND_BACKTRACE
#endif
#endif

namespace polar {
namespace sys {

using polar::basic::array_lengthof;
using polar::utils::StringSaver;
using polar::utils::RawOutStream;
using polar::utils::error_stream;

static RETSIGTYPE signal_handler(int sig);  // defined below.

static ManagedStatic<std::mutex> sg_signalsMutex;

static ManagedStatic<std::vector<std::string>> sg_filesToRemove;

/// InterruptFunction - The function to call if ctrl-c is pressed.
using InterruptFunctionType = void (*)();
InterruptFunctionType sg_interruptFunction = nullptr;

static StringRef sg_argv0;

// Signals that represent requested termination. There's no bug or failure, or
// if there is, it's not our direct responsibility. For whatever reason, our
// continued execution is no longer desirable.
static const int sg_intSigs[] =
{
   SIGHUP, SIGINT, SIGPIPE, SIGTERM, SIGUSR1, SIGUSR2
};

// Signals that represent that we have a bug, and our prompt termination has
// been ordered.
static const int sg_killSigs[] =
{
   SIGILL, SIGTRAP, SIGABRT, SIGFPE, SIGBUS, SIGSEGV, SIGQUIT
   #ifdef SIGSYS
   , SIGSYS
   #endif
   #ifdef SIGXCPU
   , SIGXCPU
   #endif
   #ifdef SIGXFSZ
   , SIGXFSZ
   #endif
   #ifdef SIGEMT
   , SIGEMT
   #endif
};

static std::atomic<unsigned> sg_numRegisteredSignals = 0;


ManagedStatic<std::vector<std::pair<void (*)(void *), void *>>>
                                                              sg_callBacksToRun;

static struct
{
   struct sigaction m_sigaction;
   int m_sigNo;
} sg_registeredSignalInfo[array_lengthof(sg_intSigs) + array_lengthof(sg_killSigs)];

static void register_handler(int signal)
{
   assert(sg_numRegisteredSignals < array_lengthof(sg_registeredSignalInfo) &&
          "Out of space for signal handlers!");

   struct sigaction newHandler;

   newHandler.sa_handler = signal_handler;
   newHandler.sa_flags = SA_NODEFER | SA_RESETHAND | SA_ONSTACK;
   sigemptyset(&newHandler.sa_mask);

   // Install the new handler, save the old one in RegisteredSignalInfo.
   sigaction(signal, &newHandler,
             &sg_registeredSignalInfo[sg_numRegisteredSignals].m_sigaction);
   sg_registeredSignalInfo[sg_numRegisteredSignals].m_sigNo = signal;
   ++sg_numRegisteredSignals;
}

#if defined(HAVE_SIGALTSTACK)
// Hold onto both the old and new alternate signal stack so that it's not
// reported as a leak. We don't make any attempt to remove our alt signal
// stack if we remove our signal handlers; that can't be done reliably if
// someone else is also trying to do the same thing.
static stack_t sg_oldAltStack;
static void* sg_newAltStackPointer;

static void create_sig_alt_stack()
{
   const size_t altStackSize = MINSIGSTKSZ + 64 * 1024;
   // If we're executing on the alternate stack, or we already have an alternate
   // signal stack that we're happy with, there's nothing for us to do. Don't
   // reduce the size, some other part of the process might need a larger stack
   // than we do.
   if (sigaltstack(nullptr, &sg_oldAltStack) != 0 ||
       sg_oldAltStack.ss_flags & SS_ONSTACK ||
       (sg_oldAltStack.ss_sp && sg_oldAltStack.ss_size >= altStackSize)) {
      return;
   }
   stack_t altStack = {};
   altStack.ss_sp = static_cast<char *>(polar::utils::safe_malloc(altStackSize));
   sg_newAltStackPointer = altStack.ss_sp; // Save to avoid reporting a leak.
   altStack.ss_size = altStackSize;
   if (sigaltstack(&altStack, &sg_oldAltStack) != 0) {
      free(altStack.ss_sp);
   }
}
#else
static void create_sig_alt_stack() {}
#endif

void register_handlers()
{
   std::lock_guard lockGuard(*sg_signalsMutex);

   // If the handlers are already registered, we're done.
   if (sg_numRegisteredSignals != 0) {
      return;
   }

   // Create an alternate stack for signal handling. This is necessary for us to
   // be able to reliably handle signals due to stack overflow.
   create_sig_alt_stack();

   for (auto sig : sg_intSigs) {
      register_handler(sig);
   }
   for (auto sig : sg_killSigs) {
      register_handler(sig);
   }
}

void unregister_handlers()
{
   // Restore all of the signal handlers to how they were before we showed up.
   for (unsigned i = 0, e = sg_numRegisteredSignals.load(); i != e; ++i) {
      sigaction(sg_registeredSignalInfo[i].m_sigNo,
                &sg_registeredSignalInfo[i].m_sigaction, nullptr);
      --sg_numRegisteredSignals;
   }
}

/// Process the sg_filesToRemove list.
void remove_files_to_remove()
{
   // Avoid constructing ManagedStatic in the signal handler.
   // If sg_filesToRemove is not constructed, there are no files to remove.
   if (!sg_filesToRemove.isConstructed()) {
      return;
   }
   // We avoid iterators in case of debug iterators that allocate or release
   // memory.
   std::vector<std::string>& FilesToRemoveRef = *sg_filesToRemove;
   for (unsigned i = 0, e = FilesToRemoveRef.size(); i != e; ++i) {
      const char *path = FilesToRemoveRef[i].c_str();

      // Get the status so we can determine if it's a file or directory. If we
      // can't stat the file, ignore it.
      struct stat buf;
      if (stat(path, &buf) != 0) {
         continue;
      }
      // If this is not a regular file, ignore it. We want to prevent removal of
      // special files like /dev/null, even if the compiler is being run with the
      // super-user permissions.
      if (!S_ISREG(buf.st_mode)) {
         continue;
      }
      // Otherwise, remove the file. We ignore any errors here as there is nothing
      // else we can do.
      unlink(path);
   }
}

void run_signal_handlers()
{
   if (!sg_callBacksToRun.isConstructed()) {
      return;
   }
   for (auto &iter : *sg_callBacksToRun) {
      iter.first(iter.second);
   }
   sg_callBacksToRun->clear();
}


// The signal handler that runs.
RETSIGTYPE signal_handler(int sig)
{
   // Restore the signal behavior to default, so that the program actually
   // crashes when we return and the signal reissues.  This also ensures that if
   // we crash in our signal handler that the program will terminate immediately
   // instead of recursing in the signal handler.
   unregister_handlers();

   // Unmask all potentially blocked kill signals.
   sigset_t sigMask;
   sigfillset(&sigMask);
   sigprocmask(SIG_UNBLOCK, &sigMask, nullptr);

   {
      std::unique_lock<std::mutex> lockGuard(*sg_signalsMutex);
      remove_files_to_remove();

      if (std::find(std::begin(sg_intSigs), std::end(sg_intSigs), sig)
          != std::end(sg_intSigs)) {
         if (sg_interruptFunction) {
            void (*ifunc)() = sg_interruptFunction;
            lockGuard.unlock();
            sg_interruptFunction = nullptr;
            ifunc();        // run the interrupt function.
            return;
         }
         lockGuard.unlock();
         raise(sig);   // Execute the default handler.
         return;
      }
   }

   // Otherwise if it is a fault (like SEGV) run any handler.
   run_signal_handlers();

#ifdef __s390__
   // On S/390, certain signals are delivered with PSW Address pointing to
   // *after* the faulting instruction.  Simply returning from the signal
   // handler would continue execution after that point, instead of
   // re-raising the signal.  Raise the signal manually in those cases.
   if (sig == SIGILL || sig == SIGFPE || sig == SIGTRAP) {
      raise(sig);
   }
#endif
}

void run_interrupt_handlers()
{
   std::lock_guard lockGuard(*sg_signalsMutex);
   remove_files_to_remove();
}

void set_interrupt_function(InterruptFunctionType func)
{
   {
      std::lock_guard lockGuard(*sg_signalsMutex);
      sg_interruptFunction = func;
   }
   register_handlers();
}

// The public API
bool remove_file_on_signal(StringRef filename, std::string* errMsg)
{
   {
      std::lock_guard lockGuard(*sg_signalsMutex);
      sg_filesToRemove->push_back(filename);
   }
   register_handlers();
   return false;
}

// The public API
void dont_remove_file_on_signal(StringRef filename)
{
   std::lock_guard lockGuard(*sg_signalsMutex);
   std::vector<std::string>::reverse_iterator reverseIter =
         find(polar::basic::reverse(*sg_filesToRemove), filename);
   std::vector<std::string>::iterator iter = sg_filesToRemove->end();
   if (reverseIter != sg_filesToRemove->rend()) {
      iter = sg_filesToRemove->erase(reverseIter.base() - 1);
   }
}

/// AddSignalHandler - Add a function to be called when a signal is delivered
/// to the process.  The handler can have a cookie passed to it to identify
/// what instance of the handler it is.
void add_signal_handler(SignalHandlerCallback funcPtr,
                        void *cookie)
{
   sg_callBacksToRun->push_back(std::make_pair(funcPtr, cookie));
   register_handlers();
}

#if defined(HAVE_BACKTRACE) && ENABLE_BACKTRACES && HAVE_LINK_H &&    \
   (defined(__linux__) || defined(__FreeBSD__) ||                             \
   defined(__FreeBSD_kernel__) || defined(__NetBSD__))
struct DlIteratePhdrData
{
   void **m_stackTrace;
   int m_depth;
   bool m_first;
   const char **m_modules;
   intptr_t *m_offsets;
   const char *m_mainExecName;
};

int dl_iterate_phdr_cb(dl_phdr_info *info, size_t size, void *arg)
{
   DlIteratePhdrData *data = (DlIteratePhdrData*)arg;
   const char *name = data->m_first ? data->m_mainExecName : info->dlpi_name;
   data->m_first = false;
   for (int i = 0; i < info->dlpi_phnum; i++) {
      const auto *phdr = &info->dlpi_phdr[i];
      if (phdr->p_type != PT_LOAD) {
         continue;
      }
      intptr_t beg = info->dlpi_addr + phdr->p_vaddr;
      intptr_t end = beg + phdr->p_memsz;
      for (int j = 0; j < data->m_depth; j++) {
         if (data->m_modules[j]) {
            continue;
         }
         intptr_t addr = (intptr_t)data->m_stackTrace[j];
         if (beg <= addr && addr < end) {
            data->m_modules[j] = name;
            data->m_offsets[j] = addr - info->dlpi_addr;
         }
      }
   }
   return 0;
}

/// If this is an ELF platform, we can find all loaded modules and their virtual
/// addresses with dl_iterate_phdr.
bool find_modules_and_offsets(void **stackTrace, int depth,
                              const char **modules, intptr_t *offsets,
                              const char *mainExecutableName,
                              StringSaver &strPool)
{
   DlIteratePhdrData data = {stackTrace, depth,   true,
                             modules,    offsets, mainExecutableName};
   dl_iterate_phdr(dl_iterate_phdr_cb, &data);
   return true;
}
#else
/// This platform does not have dl_iterate_phdr, so we do not yet know how to
/// find all loaded DSOs.
bool find_modules_and_offsets(void **stackTrace, int depth,
                              const char **modules, intptr_t *offsets,
                              const char *mainExecutableName,
                              StringSaver &strPool)
{
   return false;
}
#endif // defined(HAVE_BACKTRACE) && ENABLE_BACKTRACES && ...

#if ENABLE_BACKTRACES && defined(HAVE__UNWIND_BACKTRACE)
int unwind_backtrace(void **stackTrace, int maxEntries)
{
   if (maxEntries < 0)
      return 0;

   // Skip the first frame ('unwind_backtrace' itself).
   int entries = -1;

   auto handleFrame = [&](_Unwind_Context *Context) -> _Unwind_Reason_Code {
      // Apparently we need to detect reaching the end of the stack ourselves.
      void *ip = (void *)_Unwind_GetIP(Context);
      if (!ip) {
         return _URC_END_OF_STACK;
      }
      assert(entries < maxEntries && "recursively called after END_OF_STACK?");
      if (entries >= 0) {
         stackTrace[entries] = ip;
      }

      if (++entries == maxEntries) {
         return _URC_END_OF_STACK;
      }
      return _URC_NO_REASON;
   };

   _Unwind_Backtrace(
            [](_Unwind_Context *context, void *handler) {
      return (*static_cast<decltype(handleFrame) *>(handler))(context);
   },
   static_cast<void *>(&handleFrame));
   return std::max(entries, 0);
}
#endif

// In the case of a program crash or fault, print out a stack trace so that the
// user has an indication of why and where we died.
//
// On glibc systems we have the 'backtrace' function, which works nicely, but
// doesn't demangle symbols.
void print_stacktrace(RawOutStream &outstream) {
#if ENABLE_BACKTRACES
   static void *stackTrace[256];
   int depth = 0;
#if defined(HAVE_BACKTRACE)
   // Use backtrace() to output a backtrace on Linux systems with glibc.
   if (!depth) {
      depth = backtrace(stackTrace, static_cast<int>(array_lengthof(stackTrace)));
   }
#endif
#if defined(HAVE__UNWIND_BACKTRACE)
   // Try _Unwind_Backtrace() if backtrace() failed.
   if (!depth) {
      depth = unwind_backtrace(stackTrace,
                               static_cast<int>(array_lengthof(stackTrace)));
   }
#endif
   if (!depth) {
      return;
   }
   if (printSymbolizedstackTrace(sg_argv0, stackTrace, depth, outstream)) {
      return;
   }
#if HAVE_DLFCN_H && HAVE_DLADDR
   int width = 0;
   for (int i = 0; i < depth; ++i) {
      Dl_info dlinfo;
      dladdr(stackTrace[i], &dlinfo);
      const char* name = strrchr(dlinfo.dli_fname, '/');

      int nwidth;
      if (!name) nwidth = strlen(dlinfo.dli_fname);
      else       nwidth = strlen(name) - 1;

      if (nwidth > width) width = nwidth;
   }

   for (int i = 0; i < depth; ++i) {
      Dl_info dlinfo;
      dladdr(stackTrace[i], &dlinfo);

      outstream << format("%-2d", i);

      const char* name = strrchr(dlinfo.dli_fname, '/');
      if (!name) outstream << format(" %-*s", width, dlinfo.dli_fname);
      else       outstream << format(" %-*s", width, name+1);

      outstream << format(" %#0*lx", (int)(sizeof(void*) * 2) + 2,
                          (unsigned long)stackTrace[i]);

      if (dlinfo.dli_sname != nullptr) {
         outstream << ' ';
         int res;
         // @TODO need ?
         //         char* d = itaniumDemangle(dlinfo.dli_sname, nullptr, nullptr, &res);
         char* d = nullptr;
         if (!d) {
            outstream << dlinfo.dli_sname;
         } else {
            outstream << d;
         }
         free(d);

         // FIXME: When we move to C++11, use %t length modifier. It's not in
         // C++03 and causes gcc to issue warnings. Losing the upper 32 bits of
         // the stack offset for a stack dump isn't likely to cause any problems.
         outstream << polar::utils::format(" + %u",(unsigned)((char*)stackTrace[i]-
                                                              (char*)dlinfo.dli_saddr));
      }
      outstream << '\n';
   }
#elif defined(HAVE_BACKTRACE)
   backtrace_symbols_fd(stackTrace, depth, STDERR_FILENO);
#endif
#endif
}

static void print_stack_trace_signal_handler(void *)
{
   print_stacktrace(error_stream());
}

void disable_system_dialogs_on_crash()
{}

/// When an error signal (such as SIGABRT or SIGSEGV) is delivered to the
/// process, print a stack trace and then exit.
void print_stack_trace_on_error_signal(StringRef argv0,
                                      bool disableCrashReporting)
{
   sg_argv0 = argv0;

   add_signal_handler(print_stack_trace_signal_handler, nullptr);

#if defined(__APPLE__) && ENABLE_CRASH_OVERRIDES
   // Environment variable to disable any kind of crash dialog.
   if (DisableCrashReporting || getenv("LLVM_DISABLE_CRASH_REPORT")) {
      mach_port_t self = mach_task_self();

      exception_mask_t mask = EXC_MASK_CRASH;

      kern_return_t ret = task_set_exception_ports(self,
                                                   mask,
                                                   MACH_PORT_NULL,
                                                   EXCEPTION_STATE_IDENTITY | MACH_EXCEPTION_CODES,
                                                   THREAD_STATE_NONE);
      (void)ret;
   }
#endif
}

} // sys
} // polar
