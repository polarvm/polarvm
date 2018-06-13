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

#include "polar/global/platform/Unix.h"
#include "polar/basic/adt/StringExtras.h"
#include "polar/global/Config.h"
#include "polar/utils/ErrorCode.h"
#include "polar/utils/FileSystem.h"
#include "polar/utils/Path.h"
#include "polar/utils/RawOutStream.h"
#include "polar/utils/Program.h"

#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#if HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif
#if HAVE_SIGNAL_H
#include <signal.h>
#endif
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_POSIX_SPAWN
#include <spawn.h>

#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

#if defined(__APPLE__) && !(defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE)
#define USE_NSGETENVIRON 1
#else
#define USE_NSGETENVIRON 0
#endif

#if !USE_NSGETENVIRON
extern char **environ;
#else
#include <crt_externs.h> // _NSGetEnviron
#endif
#endif

namespace polar {
namespace sys {

using polar::basic::SmallVector;
using polar::basic::SmallString;

ProcessInfo::ProcessInfo()
   : m_pid(0), m_returnCode(0)
{}

OptionalError<std::string> find_program_by_name(StringRef name,
                                                ArrayRef<StringRef> paths) {
   assert(!name.empty() && "Must have a name!");
   // Use the given path verbatim if it contains any slashes; this matches
   // the behavior of sh(1) and friends.
   if (name.find('/') != StringRef::npos) {
      return std::string(name);
   }
   SmallVector<StringRef, 16> environmentPaths;
   if (paths.empty()) {
      if (const char *pathEnv = std::getenv("PATH")) {
         polar::basic::split_string(pathEnv, environmentPaths, ":");
         paths = environmentPaths;
      }
   }
   for (auto path : paths) {
      if (path.empty()) {
         continue;
      }
      // Check to see if this first directory contains the executable...
      SmallString<128> filePath(path);
      fs::path::append(filePath, name);
      if (fs::can_execute(filePath.getCStr())) {
         return std::string(filePath.getStr()); // Found the executable!
      }
   }
   return ErrorCode::no_such_file_or_directory;
}

namespace {

bool redirect_io(std::optional<StringRef> path, int fd, std::string* errMsg)
{
   if (!path) {// Noop
      return false;
   }
   std::string file;
   if (path->empty()) {
      // Redirect empty paths to /dev/null
      file = "/dev/null";
   } else {
      file = *path;
   }

   // Open the file
   int inFD = open(file.c_str(), fd == 0 ? O_RDONLY : O_WRONLY|O_CREAT, 0666);
   if (inFD == -1) {
      make_error_msg(errMsg, "Cannot open file '" + file + "' for "
                     + (fd == 0 ? "input" : "output"));
      return true;
   }

   // Install it as the requested FD
   if (dup2(inFD, fd) == -1) {
      make_error_msg(errMsg, "Cannot dup2");
      close(inFD);
      return true;
   }
   close(inFD);      // Close the original FD
   return false;
}

#ifdef HAVE_POSIX_SPAWN
bool redirect_io_ps(const std::string *path, int fd, std::string *errMsg,
                    posix_spawn_file_actions_t *fileActions)
{
   if (!path) {// Noop
      return false;
   }
   const char *file;
   if (path->empty()) {
      // Redirect empty paths to /dev/null
      file = "/dev/null";
   } else {
      file = path->c_str();
   }
   if (int error = posix_spawn_file_actions_addopen(
          fileActions, fd, file,
          fd == 0 ? O_RDONLY : O_WRONLY | O_CREAT, 0666)) {
      return make_error_msg(errMsg, "Cannot dup2", error);
   }
   return false;
}
#endif

void timeout_handler(int sig)
{
}

void set_memory_limits(unsigned size)
{
#if HAVE_SYS_RESOURCE_H && HAVE_GETRLIMIT && HAVE_SETRLIMIT
   struct rlimit r;
   __typeof__ (r.rlim_cur) limit = (__typeof__ (r.rlim_cur)) (size) * 1048576;

   // Heap size
   getrlimit (RLIMIT_DATA, &r);
   r.rlim_cur = limit;
   setrlimit (RLIMIT_DATA, &r);
#ifdef RLIMIT_RSS
   // Resident set size.
   getrlimit (RLIMIT_RSS, &r);
   r.rlim_cur = limit;
   setrlimit (RLIMIT_RSS, &r);
#endif
#endif
}

bool execute(ProcessInfo &processInfo, StringRef program, const char **args,
             const char **envp, ArrayRef<std::optional<StringRef>> redirects,
             unsigned memoryLimit, std::string *errMsg)
{
   if (!fs::exists(program)) {
      if (errMsg) {
         *errMsg = std::string("Executable \"") + program.getStr() +
               std::string("\" doesn't exist!");
      }
      return false;
   }

   // If this OS has posix_spawn and there is no memory limit being implied, use
   // posix_spawn.  It is more efficient than fork/exec.
#ifdef HAVE_POSIX_SPAWN
   if (memoryLimit == 0) {
      posix_spawn_file_actions_t fileActionsStore;
      posix_spawn_file_actions_t *fileActions = nullptr;

      // If we call posix_spawn_file_actions_addopen we have to make sure the
      // c strings we pass to it stay alive until the call to posix_spawn,
      // so we copy any StringRefs into this variable.
      std::string redirectsStorage[3];

      if (!redirects.empty()) {
         assert(redirects.getSize() == 3);
         std::string *redirectsStr[3] = {nullptr, nullptr, nullptr};
         for (int index = 0; index < 3; ++index) {
            if (redirects[index]) {
               redirectsStorage[index] = *redirects[index];
               redirectsStr[index] = &redirectsStorage[index];
            }
         }

         fileActions = &fileActionsStore;
         posix_spawn_file_actions_init(fileActions);

         // Redirect stdin/stdout.
         if (redirect_io_ps(redirectsStr[0], 0, errMsg, fileActions) ||
             redirect_io_ps(redirectsStr[1], 1, errMsg, fileActions)) {
            return false;
         }

         if (!redirects[1] || !redirects[2] || *redirects[1] != *redirects[2]) {
            // Just redirect stderr
            if (redirect_io_ps(redirectsStr[2], 2, errMsg, fileActions)) {
               return false;
            }

         } else {
            // If stdout and stderr should go to the same place, redirect stderr
            // to the FD already open for stdout.
            if (int error = posix_spawn_file_actions_adddup2(fileActions, 1, 2)) {
               return !make_error_msg(errMsg, "Can't redirect stderr to stdout", error);
            }
         }
      }

      if (!envp) {
#if !USE_NSGETENVIRON
         envp = const_cast<const char **>(environ);
#else
         // environ is missing in dylibs.
         envp = const_cast<const char **>(*_NSGetEnviron());
#endif
      }

      // Explicitly initialized to prevent what appears to be a valgrind false
      // positive.
      pid_t pid = 0;
      int error = posix_spawn(&pid, program.getStr().c_str(), fileActions,
                              /*attrp*/nullptr, const_cast<char **>(args),
                              const_cast<char **>(envp));

      if (fileActions) {
         posix_spawn_file_actions_destroy(fileActions);
      }

      if (error) {
         return !make_error_msg(errMsg, "posix_spawn failed", error);
      }
      processInfo.m_pid = pid;
      return true;
   }
#endif

   // Create a child process.
   int child = fork();
   switch (child) {
   // An error occurred:  Return to the caller.
   case -1:
      make_error_msg(errMsg, "Couldn't fork");
      return false;

      // Child process: Execute the program.
   case 0: {
      // Redirect file descriptors...
      if (!redirects.empty()) {
         // Redirect stdin
         if (redirect_io(redirects[0], 0, errMsg)) { return false; }
         // Redirect stdout
         if (redirect_io(redirects[1], 1, errMsg)) { return false; }
         if (redirects[1] && redirects[2] && *redirects[1] == *redirects[2]) {
            // If stdout and stderr should go to the same place, redirect stderr
            // to the FD already open for stdout.
            if (-1 == dup2(1,2)) {
               make_error_msg(errMsg, "Can't redirect stderr to stdout");
               return false;
            }
         } else {
            // Just redirect stderr
            if (redirect_io(redirects[2], 2, errMsg)) {
               return false;
            }
         }
      }

      // Set memory limits
      if (memoryLimit != 0) {
         set_memory_limits(memoryLimit);
      }

      // Execute!
      std::string pathStr = program;
      if (envp != nullptr) {
         execve(pathStr.c_str(),
                const_cast<char **>(args),
                const_cast<char **>(envp));
      } else {
         execv(pathStr.c_str(),
               const_cast<char **>(args));
      }

      // If the execve() failed, we should exit. Follow Unix protocol and
      // return 127 if the executable was not found, and 126 otherwise.
      // Use _exit rather than exit so that atexit functions and static
      // object destructors cloned from the parent process aren't
      // redundantly run, and so that any data buffered in stdio buffers
      // cloned from the parent aren't redundantly written out.
      _exit(errno == ENOENT ? 127 : 126);
   }
      // Parent process: Break out of the switch to do our processing.
   default:
      break;
   }

   processInfo.m_pid = child;
   return true;
}

} // anonymous namespace

std::error_code change_stdin_to_binary()
{
   // Do nothing, as Unix doesn't differentiate between text and binary.
   return std::error_code();
}

std::error_code change_stdout_to_binary()
{
   // Do nothing, as Unix doesn't differentiate between text and binary.
   return std::error_code();
}

std::error_code
write_file_with_encoding(StringRef fileName, StringRef contents,
                         WindowsEncodingMethod encoding /*unused*/)
{
   std::error_code errorCode;
   polar::utils::RawFdOutStream outStream(fileName, errorCode, fs::OpenFlags::F_Text);

   if (errorCode) {
      return errorCode;
   }
   outStream << contents;
   if (outStream.hasError()) {
      return make_error_code(ErrorCode::io_error);
   }
   return errorCode;
}

bool command_line_fits_within_system_limits(StringRef program,
                                            ArrayRef<const char *> args) {
   static long argMax = sysconf(_SC_ARG_MAX);

   // System says no practical limit.
   if (argMax == -1) {
      return true;
   }
   // Conservatively account for space required by environment variables.
   long halfArgMax = argMax / 2;

   size_t argLength = program.getSize() + 1;
   for (const char* arg : args) {
      size_t length = strlen(arg);

      // Ensure that we do not exceed the MAX_ARG_STRLEN constant on Linux, which
      // does not have a constant unlike what the man pages would have you
      // believe. Since this limit is pretty high, perform the check
      // unconditionally rather than trying to be aggressive and limiting it to
      // Linux only.
      if (length >= (32 * 4096)) {
         return false;
      }
      argLength += length + 1;
      if (argLength > size_t(halfArgMax)) {
         return false;
      }
   }
   return true;
}

} // sys
} // polar
