// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/06/08.

#ifndef POLAR_UTILS_PROGRAM_H
#define POLAR_UTILS_PROGRAM_H

#include "polar/basic/adt/ArrayRef.h"
#include "polar/basic/adt/StringRef.h"
#include "polar/global/PolarConfig.h"
#include "polar/utils/OptionalError.h"
#include <system_error>
#include <optional>

namespace polar {
namespace sys {

using polar::basic::ArrayRef;
using polar::basic::StringRef;
using polar::utils::OptionalError;

/// This is the OS-specific separator for PATH like environment variables:
// a colon on Unix or a semicolon on Windows.
#if defined(POLAR_ON_UNIX)
const char g_envPathSeparator = ':';
#elif defined (_WIN32)
const char g_envPathSeparator = ';';
#endif

/// This struct encapsulates information about a process.
struct ProcessInfo
{
#if defined(POLAR_ON_UNIX)
   typedef pid_t ProcessId;
#elif defined(_WIN32)
   typedef unsigned long ProcessId; // Must match the type of DWORD on Windows.
   typedef void * HANDLE; // Must match the type of HANDLE on Windows.
   /// The handle to the process (available on Windows only).
   HANDLE ProcessHandle;
#else
#error "ProcessInfo is not defined for this platform!"
#endif

   enum : ProcessId { InvalidPid = 0 };

   /// The process identifier.
   ProcessId m_pid;

   /// The return code, set after execution.
   int m_returnCode;

   ProcessInfo();
};

/// Find the first executable file \p Name in \p Paths.
///
/// This does not perform hashing as a shell would but instead stats each PATH
/// entry individually so should generally be avoided. Core POLAR library
/// functions and options should instead require fully specified paths.
///
/// \param Name name of the executable to find. If it contains any system
///   slashes, it will be returned as is.
/// \param Paths optional list of paths to search for \p Name. If empty it
///   will use the system PATH environment instead.
///
/// \returns The fully qualified path to the first \p Name in \p Paths if it
///   exists. \p Name if \p Name has slashes in it. Otherwise an error.
OptionalError<std::string>
find_program_by_name(StringRef name, ArrayRef<StringRef> paths = {});

// These functions change the specified standard stream (stdin or stdout) to
// binary mode. They return errc::success if the specified stream
// was changed. Otherwise a platform dependent error is returned.
std::error_code change_stdin_to_binary();
std::error_code change_stdout_to_binary();

/// This function executes the program using the arguments provided.  The
/// invoked program will inherit the stdin, stdout, and stderr file
/// descriptors, the environment and other configuration settings of the
/// invoking program.
/// This function waits for the program to finish, so should be avoided in
/// library functions that aren't expected to block. Consider using
/// ExecuteNoWait() instead.
/// \returns an integer result code indicating the status of the program.
/// A zero or positive value indicates the result code of the program.
/// -1 indicates failure to execute
/// -2 indicates a crash during execution or timeout
int execute_and_wait(
      StringRef program, ///< Path of the program to be executed. It is
      ///< presumed this is the result of the findProgramByName method.
      const char **args, ///< A vector of strings that are passed to the
      ///< program.  The first element should be the name of the program.
      ///< The list *must* be terminated by a null char* entry.
      const char **env = nullptr, ///< An optional vector of strings to use for
      ///< the program's environment. If not provided, the current program's
      ///< environment will be used.
      ArrayRef<std::optional<StringRef>> redirects = {}, ///<
      ///< An array of optional paths. Should have a size of zero or three.
      ///< If the array is empty, no redirections are performed.
      ///< Otherwise, the inferior process's stdin(0), stdout(1), and stderr(2)
      ///< will be redirected to the corresponding paths, if the optional path
      ///< is present (not \c polar::basic::None).
      ///< When an empty path is passed in, the corresponding file descriptor
      ///< will be disconnected (ie, /dev/null'd) in a portable way.
      unsigned secondsToWait = 0, ///< If non-zero, this specifies the amount
      ///< of time to wait for the child process to exit. If the time
      ///< expires, the child is killed and this call returns. If zero,
      ///< this function will wait until the child finishes or forever if
      ///< it doesn't.
      unsigned memoryLimit = 0, ///< If non-zero, this specifies max. amount
      ///< of memory can be allocated by process. If memory usage will be
      ///< higher limit, the child is killed and this call returns. If zero
      ///< - no memory limit.
      std::string *errMsg = nullptr, ///< If non-zero, provides a pointer to a
      ///< string instance in which error messages will be returned. If the
      ///< string is non-empty upon return an error occurred while invoking the
      ///< program.
      bool *executionFailed = nullptr);

/// Similar to ExecuteAndWait, but returns immediately.
/// @returns The \see ProcessInfo of the newly launced process.
/// \note On Microsoft Windows systems, users will need to either call
/// \see Wait until the process finished execution or win32 CloseHandle() API
/// on ProcessInfo.ProcessHandle to avoid memory leaks.
ProcessInfo execute_no_wait(StringRef program, const char **args,
                            const char **env = nullptr,
                            ArrayRef<std::optional<StringRef>> redirects = {},
                            unsigned memoryLimit = 0,
                            std::string *errMsg = nullptr,
                            bool *executionFailed = nullptr);

/// Return true if the given arguments fit within system-specific
/// argument length limits.
bool command_line_fits_within_system_limits(StringRef program,
                                           ArrayRef<const char *> args);

/// File encoding options when writing contents that a non-UTF8 tool will
/// read (on Windows systems). For UNIX, we always use UTF-8.
enum WindowsEncodingMethod
{
   /// UTF-8 is the POLAR native encoding, being the same as "do not perform
   /// encoding conversion".
   WEM_UTF8,
   WEM_CurrentCodePage,
   WEM_UTF16
};

/// Saves the UTF8-encoded \p contents string into the file \p FileName
/// using a specific encoding.
///
/// This write file function adds the possibility to choose which encoding
/// to use when writing a text file. On Windows, this is important when
/// writing files with internationalization support with an encoding that is
/// different from the one used in POLAR (UTF-8). We use this when writing
/// response files, since GCC tools on MinGW only understand legacy code
/// pages, and VisualStudio tools only understand UTF-16.
/// For UNIX, using different encodings is silently ignored, since all tools
/// work well with UTF-8.
/// This function assumes that you only use UTF-8 *text* data and will convert
/// it to your desired encoding before writing to the file.
///
/// FIXME: We use EM_CurrentCodePage to write response files for GNU tools in
/// a MinGW/MinGW-w64 environment, which has serious flaws but currently is
/// our best shot to make gcc/ld understand international characters. This
/// should be changed as soon as binutils fix this to support UTF16 on mingw.
///
/// \returns non-zero error_code if failed
std::error_code
write_file_with_encoding(StringRef fileName, StringRef contents,
                         WindowsEncodingMethod encoding = WEM_UTF8);

/// This function waits for the process specified by \p PI to finish.
/// \returns A \see ProcessInfo struct with Pid set to:
/// \li The process id of the child process if the child process has changed
/// state.
/// \li 0 if the child process has not changed state.
/// \note Users of this function should always check the ReturnCode member of
/// the \see ProcessInfo returned from this function.
ProcessInfo wait(
      const ProcessInfo &processInfo, ///< The child process that should be waited on.
      unsigned secondsToWait, ///< If non-zero, this specifies the amount of
      ///< time to wait for the child process to exit. If the time expires, the
      ///< child is killed and this function returns. If zero, this function
      ///< will perform a non-blocking wait on the child process.
      bool waitUntilTerminates, ///< If true, ignores \p SecondsToWait and waits
      ///< until child has terminated.
      std::string *errMsg = nullptr ///< If non-zero, provides a pointer to a
      ///< string instance in which error messages will be returned. If the
      ///< string is non-empty upon return an error occurred while invoking the
      ///< program.
      );

} // sys
} // polar

#endif // POLAR_UTILS_PROGRAM_H
