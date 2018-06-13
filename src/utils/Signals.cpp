// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/06/09.

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

#include "polar/utils/Signals.h"
#include "polar/basic/adt/StlExtras.h"
#include "polar/basic/adt/StringRef.h"
#include "polar/global/PolarConfig.h"
#include "polar/global/ManagedStatic.h"
#include "polar/utils/OptionalError.h"
#include "polar/utils/FileSystem.h"
#include "polar/utils/FileUtils.h"
#include "polar/utils/Format.h"
#include "polar/utils/Path.h"

#include "polar/utils/MemoryBuffer.h"
#include "polar/utils/Program.h"
#include "polar/utils/StringSaver.h"
#include "polar/utils/RawOutStream.h"
#include "polar/utils/Options.h"
#include <vector>

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

namespace polar {
namespace sys {

using polar::utils::report_fatal_error;
using polar::utils::StringSaver;
using polar::utils::FormattedNumber;
using polar::utils::BumpPtrAllocator;
using polar::utils::RawFdOutStream;
using polar::utils::FileRemover;
using polar::utils::MemoryBuffer;
using polar::basic::SmallString;
using polar::basic::SmallVector;

// Use explicit storage to avoid accessing cl::opt in a signal handler.
static bool sg_disableSymbolicationFlag = false;
static cmd::Opt<bool, true>
sg_disableSymbolication("disable-symbolication",
                        cmd::Desc("Disable symbolizing crash backtraces."),
                        cmd::location(DisableSymbolicationFlag), cmd::Hidden);

// Callbacks to run in signal handler must be lock-free because a signal handler
// could be running as we add new callbacks. We don't add unbounded numbers of
// callbacks, an array is therefore sufficient.
struct CallbackAndCookie
{
   sys::SignalHandlerCallback m_callback;
   void *m_cookie;
   enum class Status
   {
      Empty,
      Initializing,
      Initialized,
      Executing
   };
   std::atomic<Status> m_flag;
};

static constexpr size_t sg_maxSignalHandlerCallbacks = 8;
static CallbackAndCookie sg_callBacksToRun[sg_maxSignalHandlerCallbacks];

// Signal-safe.
void run_signal_handlers()
{
   for (size_t index = 0; index < sg_maxSignalHandlerCallbacks; ++index) {
      auto &runMe = sg_callBacksToRun[index];
      auto expected = CallbackAndCookie::Status::Initialized;
      auto desired = CallbackAndCookie::Status::Executing;
      if (!runMe.m_flag.compare_exchange_strong(expected, desired)) {
         continue;
      }
      (*runMe.m_callback)(runMe.m_cookie);
      runMe.m_callback = nullptr;
      runMe.m_cookie = nullptr;
      runMe.m_flag.store(CallbackAndCookie::Status::Empty);
   }
}

// Signal-safe.
void insert_signal_handler(sys::SignalHandlerCallback funcPtr,
                           void *cookie)
{
   for (size_t index = 0; index < sg_maxSignalHandlerCallbacks; ++index) {
      auto &setMe = sg_callBacksToRun[index];
      auto expected = CallbackAndCookie::Status::Empty;
      auto desired = CallbackAndCookie::Status::Initializing;
      if (!setMe.m_flag.compare_exchange_strong(expected, desired)) {
         continue;
      }
      setMe.m_callback = funcPtr;
      setMe.m_cookie = cookie;
      setMe.m_flag.store(CallbackAndCookie::Status::Initialized);
      return;
   }
   report_fatal_error("too many signal callbacks already registered");
}

bool find_modules_and_offsets(void **stackTrace, int depth,
                              const char **modules, intptr_t *offsets,
                              const char *mainExecutableName,
                              StringSaver &strPool);

/// Format a pointer value as hexadecimal. Zero pad it out so its always the
/// same width.
FormattedNumber format_ptr(void *pc)
{
   // Each byte is two hex digits plus 2 for the 0x prefix.
   unsigned ptrWidth = 2 + 2 * sizeof(void *);
   return polar::utils::format_hex((uint64_t)pc, ptrWidth);
}

/// Helper that launches llvm-symbolizer and symbolizes a backtrace.
POLAR_ATTRIBUTE_USED
bool print_symbolized_stack_trace(StringRef argv0, void **stackTrace,
                                  int depth, RawOutStream &outstream)
{
   if (sg_disableSymbolicationFlag) {
      return false;
   }
   // Don't recursively invoke the polar-symbolizer binary.
   if (argv0.find("polar-symbolizer") != std::string::npos) {
      return false;
   }
   // FIXME: Subtract necessary number from StackTrace entries to turn return addresses
   // into actual instruction addresses.
   // Use llvm-symbolizer tool to symbolize the stack traces. First look for it
   // alongside our binary, then in $PATH.
   OptionalError<std::string> polarPHPSymbolizerPathOrErr = std::error_code();
   if (!argv0.empty()) {
      StringRef parent =  polar::fs::path::parent_path(argv0);
      if (!parent.empty())
         polarPHPSymbolizerPathOrErr = polar::sys::find_program_by_name("polar-symbolizer", parent);
   }
   if (!polarPHPSymbolizerPathOrErr) {
      polarPHPSymbolizerPathOrErr = polar::sys::find_program_by_name("polar-symbolizer");
   }
   if (!polarPHPSymbolizerPathOrErr) {
      return false;
   }

   const std::string &polarPHPSymbolizerPath = *polarPHPSymbolizerPathOrErr;

   // If we don't know argv0 or the address of main() at this point, try
   // to guess it anyway (it's possible on some platforms).
   std::string mainExecutableName =
         argv0.empty() ? polar::fs::get_main_executable(nullptr, nullptr)
                       : (std::string)argv0;
   BumpPtrAllocator allocator;
   StringSaver strPool(allocator);
   std::vector<const char *> modules(depth, nullptr);
   std::vector<intptr_t> offsets(depth, 0);
   if (!findModulesAndOffsets(stackTrace, depth, modules.data(), offsets.data(),
                              mainExecutableName.c_str(), strPool)) {
      return false;
   }
   int inputFD;
   SmallString<32> inputFile;
   SmallString<32> outputFile;
   polar::fs::create_temporary_file("symbolizer-input", "", inputFD, inputFile);
   polar::fs::create_temporary_file("symbolizer-output", "", outputFile);
   FileRemover inputRemover(inputFile.c_str());
   FileRemover outputRemover(outputFile.c_str());

   {
      RawFdOutStream innput(inputFD, true);
      for (int i = 0; i < depth; i++) {
         if (modules[i]) {
            input << modules[i] << " " << (void*)offsets[i] << "\n";
         }
      }
   }

   std::optional<StringRef> redirects[] = {inputFile.str(), outputFile.str(), std::nullopt};
   const char *args[] = {"polar-symbolizer", "--functions=linkage", "--inlining",
                      #ifdef _WIN32
                         // Pass --relative-address on Windows so that we don't
                         // have to add ImageBase from PE file.
                         // FIXME: Make this the default for llvm-symbolizer.
                         "--relative-address",
                      #endif
                         "--demangle", nullptr};
   int runResult =
         polar::sys::execute_and_wait(polarPHPSymbolizerPath, args, nullptr, redirects);
   if (runResult != 0)
      return false;

   // This report format is based on the sanitizer stack trace printer.  See
   // sanitizer_stacktrace_printer.cc in compiler-rt.
   auto outputBuf = MemoryBuffer::getFile(outputFile.c_str());
   if (!outputBuf) {
      return false;
   }
   StringRef output = outputBuf.get()->getBuffer();
   SmallVector<StringRef, 32> lines;
   output.split(lines, "\n");
   auto curLine = lines.begin();
   int frameno = 0;
   for (int i = 0; i < depth; i++) {
      if (!modules[i]) {
         outstream << '#' << frameno++ << ' ' << format_ptr(stackTrace[i]) << '\n';
         continue;
      }
      // Read pairs of lines (function name and file/line info) until we
      // encounter empty line.
      for (;;) {
         if (curLine == lines.end()) {
            return false;
         }
         StringRef functionName = *curLine++;
         if (functionName.empty()) {
            break;
         }
         outstream << '#' << frameno++ << ' ' << format_ptr(stackTrace[i]) << ' ';
         if (!functionName.startswith("??")) {
            outstream << functionName << ' ';
         }
         if (curLine == lines.end()) {
            return false;
         }
         StringRef fileLineInfo = *curLine++;
         if (!fileLineInfo.startsWith("??")) {
            outstream << fileLineInfo;
         } else {
            outstream << "(" << modules[i] << '+' << format_hex(offsets[i], 0) << ")";
         }
         outstream << "\n";
      }
   }
   return true;
}

} // sys
} // polar
