// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/06/04.

#ifndef POLAR_UTILS_DJB_HASH_H
#define POLAR_UTILS_DJB_HASH_H

#include "polar/basic/adt/StringRef.h"

namespace polar {
namespace utils {

/// The Bernstein hash function used by the DWARF accelerator tables.
inline uint32_t djb_hash(StringRef buffer, uint32_t hash = 5381)
{
   for (unsigned char c : buffer.getBytes()) {
      hash = (hash << 5) + hash + c;
   }
   return hash;
}

/// Computes the Bernstein hash after folding the input according to the Dwarf 5
/// standard case folding rules.
uint32_t case_folding_djb_hash(StringRef buffer, uint32_t hash = 5381);

} // utils
} // polar

#endif // POLAR_UTILS_DJB_HASH_H
