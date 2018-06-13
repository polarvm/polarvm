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

#ifndef POLAR_UTILS_STRING_SAVER_H
#define POLAR_UTILS_STRING_SAVER_H

#include "polar/basic/adt/StringRef.h"
#include "polar/basic/adt/Twine.h"
#include "polar/utils/Allocator.h"

namespace polar {
namespace utils {

/// Saves strings in the inheritor's stable storage and returns a
/// StringRef with a stable character pointer.
class StringSaver final
{
   BumpPtrAllocator &m_alloc;
public:
   StringSaver(BumpPtrAllocator &alloc) : m_alloc(alloc)
   {}

   StringRef save(const char *str)
   {
      return save(StringRef(str));
   }

   StringRef save(StringRef str);
   StringRef save(const Twine &str)
   {
      return save(StringRef(str.getStr()));
   }

   StringRef save(const std::string &str)
   {
      return save(StringRef(str));
   }
};

} // utils
} // polar

#endif // POLAR_UTILS_STRING_SAVER_H

