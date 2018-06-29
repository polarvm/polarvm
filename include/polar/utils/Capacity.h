// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/06/29.

#ifndef POLAR_UTILS_CAPACITY_H
#define POLAR_UTILS_CAPACITY_H

#include <cstddef>

namespace polar {
namespace utils {

template <typename T>
static inline size_t capacity_in_bytes(const T &object)
{
   // This default definition of capacity should work for things like std::vector
   // and friends.  More specialized versions will work for others.
   return object.getCapacity() * sizeof(typename T::value_type);
}

} // utils
} // polar

#endif // POLAR_UTILS_CAPACITY_H
