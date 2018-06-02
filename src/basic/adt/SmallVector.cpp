// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/06/02.

#include "polar/basic/adt/SmallVector.h"
#include "polar/utils/ErrorHandling.h"

namespace polar {
namespace basic {

/// growPod - This is an implementation of the grow() method which only works
/// on POD-like datatypes and is out of line to reduce code duplication.
void SmallVectorBase::growPod(void *firstEl, size_t minSizeInBytes,
                              size_t tsize)
{
   size_t curSizeBytes = getSizeInBytes();
   size_t newCapacityInBytes = 2 * getCapacityInBytes() + tsize; // Always grow.
   if (newCapacityInBytes < minSizeInBytes) {
      newCapacityInBytes = minSizeInBytes;
   }
   void *newElts;
   if (m_beginX == firstEl) {
      newElts = malloc(newCapacityInBytes);
      if (newElts == nullptr) {
         polar::utils::report_bad_alloc_error("Allocation of SmallVector element failed.");
      }
      // Copy the elements over.  No need to run dtors on PODs.
      memcpy(newElts, m_beginX, curSizeBytes);
   } else {
      // If this wasn't grown from the inline copy, grow the allocated space.
      newElts = realloc(m_beginX, newCapacityInBytes);
      if (newElts == nullptr) {
         polar::utils::report_bad_alloc_error("Reallocation of SmallVector element failed.");
      }
   }
   m_endX = (char*)newElts + curSizeBytes;
   m_beginX = newElts;
   m_capacityX = (char*)m_beginX + newCapacityInBytes;
}

} // basic
} // polar
