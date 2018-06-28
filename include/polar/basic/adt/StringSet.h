// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/06/28.

#ifndef POLAR_BASIC_ADT_STRING_SET_H
#define POLAR_BASIC_ADT_STRING_SET_H

#include "polar/basic/adt/StringMap.h"
#include "polar/basic/adt/StringRef.h"
#include "polar/utils/Allocator.h"
#include <cassert>
#include <initializer_list>
#include <utility>

namespace polar {
namespace basic {

/// StringSet - A wrapper for StringMap that provides set-like functionality.
template <class AllocatorTy = MallocAllocator>
class StringSet : public StringMap<char, AllocatorTy>
{
   using base = StringMap<char, AllocatorTy>;

public:
   StringSet() = default;
   StringSet(std::initializer_list<StringRef> strs) {
      for (StringRef str : strs) {
         insert(str);
      }
   }

   std::pair<typename base::iterator, bool> insert(StringRef key)
   {
      assert(!key.empty());
      return base::insert(std::make_pair(key, '\0'));
   }

   template <typename InputIter>
   void insert(const InputIter &begin, const InputIter &end)
   {
      for (auto iter = begin; iter != end; ++iter) {
         base::insert(std::make_pair(*iter, '\0'));
      }
   }
};

} // basic
} // polar

#endif // POLAR_BASIC_ADT_STRING_SET_H
