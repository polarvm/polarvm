// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/06/07.

#include "polar/utils/ErrorNumber.h"
#include "polar/global/Config.h"
#include "polar/utils/RawOutStream.h"
#include <string.h>

#if HAVE_ERRNO_H
#include <errno.h>
#endif

namespace polar {
namespace sys {

std::string get_error_str(int errnum)
{
   std::string str;
   if (errnum == 0) {
      return str;
   }
#if defined(HAVE_STRERROR_R) || HAVE_DECL_STRERROR_S
   const int m_maxErrStrLen = 2000;
   char buffer[m_maxErrStrLen];
   buffer[0] = '\0';
#endif

#ifdef HAVE_STRERROR_R
   // strerror_r is thread-safe.
#if defined(__GLIBC__) && defined(_GNU_SOURCE)
   // glibc defines its own incompatible version of strerror_r
   // which may not use the buffer supplied.
   str = strerror_r(errnum, buffer, m_maxErrStrLen - 1);
#else
   strerror_r(errnum, buffer, m_maxErrStrLen - 1);
   str = buffer;
#endif
#elif HAVE_DECL_STRERROR_S // "Windows Secure API"
   strerror_s(buffer, m_maxErrStrLen - 1, errnum);
   str = buffer;
#elif defined(HAVE_STRERROR)
   // Copy the thread un-safe result of strerror into
   // the buffer as fast as possible to minimize impact
   // of collision of strerror in multiple threads.
   str = strerror(errnum);
#else
   // Strange that this system doesn't even have strerror
   // but, oh well, just use a generic message
   RawStringOutStream stream(str);
   stream << "Error #" << errnum;
   stream.flush();
#endif
   return str;
}

#if HAVE_ERRNO_H
std::string get_error_str()
{
   return get_error_str(errno);
}
#endif  // HAVE_ERRNO_H

} // sys
} // polar
