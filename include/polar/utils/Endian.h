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

#ifndef POLAR_UTILS_ENDIAN_H
#define POLAR_UTILS_ENDIAN_H

#include "polar/utils/AlignOf.h"
#include "polar/utils/SwapByteOrder.h"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <type_traits>

namespace polar {
namespace utils {

enum class Endianness
{
   Big,
   Little,
   Native
};

// These are named values for common alignments.
static const int ALIGINED = 0;
static const int UNALIGNED = 1;

namespace internal {

/// \brief ::value is either alignment, or alignof(T) if alignment is 0.
template<class T, int alignment>
struct PickAlignment
{
   enum { value = alignment == 0 ? alignof(T) : alignment };
};

} // end namespace internal

namespace endian {

constexpr Endianness system_endianness()
{
   return sys::IsBigEndianHost ? Endianness::Big : Endianness::Little;
}

template <typename value_type>
inline value_type byte_swap(value_type value, Endianness endian)
{
   if ((endian != native) && (endian != system_endianness())) {
      polar::utils::swap_byte_order(value);
   }
   return value;
}

/// Swap the bytes of value to match the given endianness.
template<typename value_type, Endianness endian>
inline value_type byte_swap(value_type value)
{
   return byte_swap(value, endian);
}

/// Read a value of a particular endianness from memory.
template <typename value_type, std::size_t alignment>
inline value_type read(const void *memory, Endianness endian)
{
   value_type ret;
   memcpy(&ret,
          POLAR_ASSUME_ALIGNED(
             memory, (internal::PickAlignment<value_type, alignment>::value)),
          sizeof(value_type));
   return byte_swap<value_type>(ret, endian);
}

template<typename value_type,
         Endianness endian,
         std::size_t alignment>
inline value_type read(const void *memory)
{
   return read<value_type, alignment>(memory, endian);
}

/// Read a value of a particular endianness from a buffer, and increment the
/// buffer past that value.
template <typename value_type, std::size_t alignment, typename CharT>
inline value_type read_next(const CharT *&memory, Endianness endian)
{
   value_type ret = read<value_type, alignment>(memory, endian);
   memory += sizeof(value_type);
   return ret;
}

template<typename value_type, Endianness endian, std::size_t alignment,
         typename CharT>
inline value_type read_next(const CharT *&memory)
{
   return readNext<value_type, alignment, CharT>(memory, endian);
}

/// Write a value to memory with a particular endianness.
template <typename value_type, std::size_t alignment>
inline void write(void *memory, value_type value, Endianness endian)
{
   value = byte_swap<value_type>(value, endian);
   memcpy(POLAR_ASSUME_ALIGNED(
             memory, (internal::PickAlignment<value_type, alignment>::value)),
          &value, sizeof(value_type));
}

template<typename value_type,
         Endianness endian,
         std::size_t alignment>
inline void write(void *memory, value_type value)
{
   write<value_type, alignment>(memory, value, endian);
}

template <typename value_type>
using make_unsigned_t = typename std::make_unsigned<value_type>::type;

/// Read a value of a particular endianness from memory, for a location
/// that starts at the given bit offset within the first byte.
template <typename value_type, Endianness endian, std::size_t alignment>
inline value_type read_at_bit_alignment(const void *memory, uint64_t startBit)
{
   assert(startBit < 8);
   if (startBit == 0) {
      return read<value_type, endian, alignment>(memory);
   } else {
      // Read two values and compose the result from them.
      value_type val[2];
      memcpy(&val[0],
            POLAR_ASSUME_ALIGNED(
               memory, (internal::PickAlignment<value_type, alignment>::value)),
            sizeof(value_type) * 2);
      val[0] = byte_swap<value_type, endian>(val[0]);
      val[1] = byte_swap<value_type, endian>(val[1]);
      
      // Shift bits from the lower value into place.
      make_unsigned_t<value_type> lowerVal = val[0] >> startBit;
      // Mask off upper bits after right shift in case of signed type.
      make_unsigned_t<value_type> numBitsFirstVal =
            (sizeof(value_type) * 8) - startBit;
      lowerVal &= ((make_unsigned_t<value_type>)1 << numBitsFirstVal) - 1;
      
      // Get the bits from the upper value.
      make_unsigned_t<value_type> upperVal =
            val[1] & (((make_unsigned_t<value_type>)1 << startBit) - 1);
      // Shift them in to place.
      upperVal <<= numBitsFirstVal;
      
      return lowerVal | upperVal;
   }
}

/// Write a value to memory with a particular endianness, for a location
/// that starts at the given bit offset within the first byte.
template <typename value_type, Endianness endian, std::size_t alignment>
inline void write_at_bit_alignment(void *memory, value_type value,
                                   uint64_t startBit)
{
   assert(startBit < 8);
   if (startBit == 0) {
      write<value_type, endian, alignment>(memory, value);
   } else {
      // Read two values and shift the result into them.
      value_type val[2];
      memcpy(&val[0],
            POLAR_ASSUME_ALIGNED(
               memory, (internal::PickAlignment<value_type, alignment>::value)),
            sizeof(value_type) * 2);
      val[0] = byte_swap<value_type, endian>(val[0]);
      val[1] = byte_swap<value_type, endian>(val[1]);
      
      // Mask off any existing bits in the upper part of the lower value that
      // we want to replace.
      val[0] &= ((make_unsigned_t<value_type>)1 << startBit) - 1;
      make_unsigned_t<value_type> numBitsFirstVal =
            (sizeof(value_type) * 8) - startBit;
      make_unsigned_t<value_type> lowerVal = value;
      if (startBit > 0) {
         // Mask off the upper bits in the new value that are not going to go into
         // the lower value. This avoids a left shift of a negative value, which
         // is undefined behavior.
         lowerVal &= (((make_unsigned_t<value_type>)1 << numBitsFirstVal) - 1);
         // Now shift the new bits into place
         lowerVal <<= startBit;
      }
      val[0] |= lowerVal;
      
      // Mask off any existing bits in the lower part of the upper value that
      // we want to replace.
      val[1] &= ~(((make_unsigned_t<value_type>)1 << startBit) - 1);
      // Next shift the bits that go into the upper value into position.
      make_unsigned_t<value_type> upperVal = value >> numBitsFirstVal;
      // Mask off upper bits after right shift in case of signed type.
      upperVal &= ((make_unsigned_t<value_type>)1 << startBit) - 1;
      val[1] |= upperVal;
      
      // Finally, rewrite values.
      val[0] = byte_swap<value_type, endian>(val[0]);
      val[1] = byte_swap<value_type, endian>(val[1]);
      memcpy(POLAR_ASSUME_ALIGNED(
                memory, (internal::PickAlignment<value_type, alignment>::value)),
             &val[0], sizeof(value_type) * 2);
   }
}

} // end namespace endian

} // utils
} // polar

#endif // POLAR_UTILS_ENDIAN_H
