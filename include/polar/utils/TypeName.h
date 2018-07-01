// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/07/01.

#ifndef POLAR_UTILS_TYPE_NAME_H
#define POLAR_UTILS_TYPE_NAME_H

#include "polar/basic/adt/StringRef.h"

namespace polar {
namespace utils {

/// We provide a function which tries to compute the (demangled) name of a type
/// statically.
///
/// This routine may fail on some platforms or for particularly unusual types.
/// Do not use it for anything other than logging and debugging aids. It isn't
/// portable or dependendable in any real sense.
///
/// The returned StringRef will point into a static storage duration string.
/// However, it may not be null terminated and may be some strangely aligned
/// inner substring of a larger string.
template <typename DesiredTypeName>
inline StringRef getTypeName()
{
#if defined(__clang__) || defined(__GNUC__)
   StringRef name = __PRETTY_FUNCTION__;

   StringRef key = "DesiredTypeName = ";
   name = name.substr(name.find(key));
   assert(!name.empty() && "Unable to find the template parameter!");
   name = name.dropFront(key.getSize());

   assert(name.endsWith("]") && "name doesn't end in the substitution key!");
   return name.dropBack(1);
#elif defined(_MSC_VER)
   StringRef name = __FUNCSIG__;

   StringRef key = "getTypeName<";
   name = name.substr(name.find(key));
   assert(!name.empty() && "Unable to find the function name!");
   name = name.dropFront(key.getSize());

   for (StringRef prefix : {"class ", "struct ", "union ", "enum "}) {
      if (name.startswith(prefix)) {
         name = name.dropFront(prefix.size());
         break;
      }
   }
   auto anglePos = name.rfind('>');
   assert(anglePos != StringRef::npos && "Unable to find the closing '>'!");
   return name.substr(0, anglePos);
#else
   // No known technique for statically extracting a type name on this compiler.
   // We return a string that is unlikely to look like any type in polarVM.
   return "UNKNOWN_TYPE";
#endif
}

} // utils
} // polar

#endif // POLAR_UTILS_TYPE_NAME_H
