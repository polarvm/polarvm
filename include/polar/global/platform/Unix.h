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
//
// This file defines things specific to Unix implementations.
//
//===----------------------------------------------------------------------===//

#ifndef POLAR_GLOBAL_PLATFORM_UNIX_H
#define POLAR_GLOBAL_PLATFORM_UNIX_H

#include "polar/global/Config.h" // Get autoconf configuration settings
#include "polar/utils/Chrono.h"
#include "polar/utils/ErrorNumber.h"
#include <algorithm>
#include <assert.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif
#include <time.h>

#ifdef HAVE_DLFCN_H
# include <dlfcn.h>
#endif

#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif

namespace polar {

using utils::TimePoint;

/// This function builds an error message into \p ErrMsg using the \p prefix
/// string and the Unix error number given by \p errnum. If errnum is -1, the
/// default then the value of errno is used.
/// Make an error message
///
/// If the error number can be converted to a string, it will be
/// separated from prefix by ": ".
namespace {
inline bool make_error_msg(
      std::string* errorMsg, const std::string& prefix, int errnum = -1)
{
   if (!errorMsg) {
      return true;
   }
   if (errnum == -1) {
      errnum = errno;
   }
   *errorMsg = prefix + ": " + polar::sys::get_error_str(errnum);
   return true;
}
} // anonymous namespace

/// Convert a struct timeval to a duration. Note that timeval can be used both
/// as a time point and a duration. Be sure to check what the input represents.
inline std::chrono::microseconds to_duration(const struct timeval &tv)
{
   return std::chrono::seconds(tv.tv_sec) +
         std::chrono::microseconds(tv.tv_usec);
}

/// Convert a time point to struct timespec.
inline struct timespec to_time_spec(TimePoint<> timePoint)
{
   using namespace std::chrono;
   struct timespec retVal;
   retVal.tv_sec = utils::to_time_t(timePoint);
   retVal.tv_nsec = (timePoint.time_since_epoch() % seconds(1)).count();
   return retVal;
}

/// Convert a time point to struct timeval.
inline struct timeval to_time_val(TimePoint<std::chrono::microseconds> timePoint)
{
   using namespace std::chrono;

   struct timeval retVal;
   retVal.tv_sec = utils::to_time_t(timePoint);
   retVal.tv_usec = (timePoint.time_since_epoch() % seconds(1)).count();
   return retVal;
}

} // polar

#endif // POLAR_GLOBAL_PLATFORM_UNIX_H
