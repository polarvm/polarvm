// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/06/26.

#ifndef POLAR_UTILS_MEMORY_ALLOC_H
#define POLAR_UTILS_MEMORY_ALLOC_H

#include "polar/global/Global.h"
#include "polar/utils/ErrorHandling.h"
#include <cstdlib>

namespace polar {
namespace utils {

POLAR_ATTRIBUTE_RETURNS_NONNULL inline void *safe_malloc(size_t size)
{
   void *result = std::malloc(size);
   if (result == nullptr) {
      report_bad_alloc_error("Allocation failed");
   }
   return result;
}

POLAR_ATTRIBUTE_RETURNS_NONNULL inline void *safe_calloc(size_t count,
                                                         size_t size)
{
   void *result = std::calloc(count, size);
   if (result == nullptr) {
      report_bad_alloc_error("Allocation failed");
   }
   return result;
}

POLAR_ATTRIBUTE_RETURNS_NONNULL inline void *safe_realloc(void *ptr, size_t size)
{
   void *result = std::realloc(ptr, size);
   if (result == nullptr) {
      report_bad_alloc_error("Allocation failed");
   }
   return result;
}

} // utils
} // polar

#endif // POLAR_UTILS_MEMORY_ALLOC_H
