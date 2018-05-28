// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
// 
// Created by softboy on 2018/05/28.

#ifndef POLAR_UTILS_ERROR_HANDLING_H
#define POLAR_UTILS_ERROR_HANDLING_H

#include "polar/global/Global.h"
#include <string>

namespace polar {
namespace utils {

class StringRef;
class Twine;

/// An error handler callback.
using FatalErrorHandlerType = void (*)(void *userData, const std::string& reason, bool genCrashDiag);

/// install_fatal_error_handler - Installs a new error handler to be used
/// whenever a serious (non-recoverable) error is encountered by LLVM.
///
/// If no error handler is installed the default is to print the error message
/// to stderr, and call exit(1).  If an error handler is installed then it is
/// the handler's responsibility to log the message, it will no longer be
/// printed to stderr.  If the error handler returns, then exit(1) will be
/// called.
///
/// It is dangerous to naively use an error handler which throws an exception.
/// Even though some applications desire to gracefully recover from arbitrary
/// faults, blindly throwing exceptions through unfamiliar code isn't a way to
/// achieve this.
///
/// \param user_data - An argument which will be passed to the install error
/// handler.
void install_fatal_error_handler(FatalErrorHandlerType handler,
                                 void *userData = nullptr);

/// Restores default error handling behaviour.
void remove_fatal_error_handler();

/// ScopedFatalErrorHandler - This is a simple helper class which just
/// calls install_fatal_error_handler in its constructor and
/// remove_fatal_error_handler in its destructor.
struct ScopedFatalErrorHandler
{
   explicit ScopedFatalErrorHandler(FatalErrorHandlerType handler,
                                    void *userData = nullptr)
   {
      install_fatal_error_handler(handler, userData);
   }
   
   ~ScopedFatalErrorHandler()
   {
      remove_fatal_error_handler();
   }
};

/// Reports a serious error, calling any installed error handler. These
/// functions are intended to be used for error conditions which are outside
/// the control of the compiler (I/O errors, invalid user input, etc.)
///
/// If no error handler is installed the default is to print the message to
/// standard error, followed by a newline.
/// After the error handler is called this function will call exit(1), it
/// does not return.
POLAR_ATTRIBUTE_NORETURN void report_fatal_error(const char *reason,
                                                 bool genCrashDiag = true);
POLAR_ATTRIBUTE_NORETURN void report_fatal_error(const std::string &reason,
                                                 bool genCrashDiag = true);
POLAR_ATTRIBUTE_NORETURN void report_fatal_error(StringRef reason,
                                                 bool genCrashDiag = true);
POLAR_ATTRIBUTE_NORETURN void report_fatal_error(const Twine &reason,
                                                 bool genCrashDiag = true);

/// Installs a new bad alloc error handler that should be used whenever a
/// bad alloc error, e.g. failing malloc/calloc, is encountered by LLVM.
///
/// The user can install a bad alloc handler, in order to define the behavior
/// in case of failing allocations, e.g. throwing an exception. Note that this
/// handler must not trigger any additional allocations itself.
///
/// If no error handler is installed the default is to print the error message
/// to stderr, and call exit(1).  If an error handler is installed then it is
/// the handler's responsibility to log the message, it will no longer be
/// printed to stderr.  If the error handler returns, then exit(1) will be
/// called.
///
///
/// \param user_data - An argument which will be passed to the installed error
/// handler.
void install_bad_alloc_error_handler(FatalErrorHandlerType handler,
                                     void *userData = nullptr);

/// Restores default bad alloc error handling behavior.
void remove_bad_alloc_error_handler();

/// Reports a bad alloc error, calling any user defined bad alloc
/// error handler. In contrast to the generic 'report_fatal_error'
/// functions, this function is expected to return, e.g. the user
/// defined error handler throws an exception.
///
/// Note: When throwing an exception in the bad alloc handler, make sure that
/// the following unwind succeeds, e.g. do not trigger additional allocations
/// in the unwind chain.
///
/// If no error handler is installed (default), then a bad_alloc exception
/// is thrown if PolarPHP is compiled with exception support, otherwise an assertion
/// is called.
void report_bad_alloc_error(const char *reason, bool genCrashDiag = true);

/// This function calls abort(), and prints the optional message to stderr.
/// Use the polar_unreachable macro (that adds location info), instead of
/// calling this function directly.
POLAR_ATTRIBUTE_NORETURN void
unreachable_internal(const char *msg = nullptr, const char *file = nullptr,
                     unsigned line = 0);

} // utils
} // polar

/// Marks that the current location is not supposed to be reachable.
/// In !NDEBUG builds, prints the message and location info to stderr.
/// In NDEBUG builds, becomes an optimizer hint that the current location
/// is not supposed to be reachable.  On compilers that don't support
/// such hints, prints a reduced message instead.
///
/// Use this instead of assert(0).  It conveys intent more clearly and
/// allows compilers to omit some unnecessary code.
#ifndef NDEBUG
#define polar_unreachable(msg) \
   ::polar::utils::unreachable_internal(msg, __FILE__, __LINE__)
#elif defined(POLAR_BUILTIN_UNREACHABLE)
#define polar_unreachable(msg) POLAR_BUILTIN_UNREACHABLE
#else
#define polar_unreachable(msg) ::polar::unreachable_internal()
#endif

#endif // POLAR_UTILS_ERROR_HANDLING_H
