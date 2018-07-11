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

#ifndef POLAR_UTILS_REVERSE_ITERATION_H
#define POLAR_UTILS_REVERSE_ITERATION_H

#include "polar/global/AbiBreaking.h"
#include "polar/utils/PointerLikeTypeTraits.h"

namespace polar {
namespace utils {

template<class T = void *>
bool should_reverse_iterate()
{
   return internal::IsPointerLike<T>::value;
}

} // utils
} // polar

#endif // POLAR_UTILS_REVERSE_ITERATION_H
