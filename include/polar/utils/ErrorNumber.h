// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/05/31.

#ifndef POLAR_UTILS_ERROR_ERROR_NUMBER_H
#define POLAR_UTILS_ERROR_ERROR_NUMBER_H

#include <cerrno>
#include <string>
#include <type_traits>

namespace polar {
namespace sys {

/// Returns a string representation of the errno value, using whatever
/// thread-safe variant of strerror() is available.  Be sure to call this
/// immediately after the function that set errno, or errno may have been
/// overwritten by an intervening call.
std::string get_error_str();

/// Like the no-argument version above, but uses \p errnum instead of errno.
std::string get_error_str(int errnum);

template <typename FailT, typename Func, typename... Args>
inline auto retry_after_signal(const FailT &failure, const Func &func,
                               const Args &... args) -> decltype(func(args...))
{
   decltype(func(args...)) result;
   do {
      result = func(args...);
   } while (result == failure && errno == EINTR);
   return result;
}

} // sys
} // polar

#endif // POLAR_UTILS_ERROR_ERROR_NUMBER_H
