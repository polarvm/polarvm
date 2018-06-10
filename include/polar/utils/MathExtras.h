// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/05/28.

#ifndef POLAR_UTILS_MATH_EXTRAS_H
#define POLAR_UTILS_MATH_EXTRAS_H

#include "polar/global/Global.h"
#include "polar/utils/SwapByteOrder.h"
#include <algorithm>
#include <cassert>
#include <climits>
#include <cstring>
#include <limits>
#include <type_traits>

#ifdef POLAR_CC_MSVC
#include <intrin.h>
#endif

namespace polar {
namespace utils {

/// \brief The behavior an operation has on an input of 0.
enum class ZeroBehavior
{
   /// \brief The returned value is undefined.
   Undefined,
   /// \brief The returned value is numeric_limits<T>::max()
   Max,
   /// \brief The returned value is numeric_limits<T>::digits
   Width
};

namespace internal {

template <typename T, std::size_t SizeOfTypeype>
struct TrailingZerosCounter
{
   static std::size_t count(T value, ZeroBehavior) {
      if (!value) {
         return std::numeric_limits<T>::digits;
      }
      if (value & 0x1) {
         return 0;
      }
      // Bisection method.
      std::size_t zerobits = 0;
      T shift = std::numeric_limits<T>::digits >> 1;
      T mask = std::numeric_limits<T>::max() >> shift;
      while (shift) {
         if ((value & mask) == 0) {
            value >>= shift;
            zerobits |= shift;
         }
         shift >>= 1;
         mask >>= shift;
      }
      return zerobits;
   }
};

#if __GNUC__ >= 4 || defined(POLAR_CC_MSVC)
template <typename T>
struct TrailingZerosCounter<T, 4>
{
   static std::size_t count(T value, ZeroBehavior zb) {
      if (zb != ZeroBehavior::Undefined && value == 0) {
         return 32;
      }
#if __has_builtin(__builtin_ctz) || POLAR_GNUC_PREREQ(4, 0, 0)
      return __builtin_ctz(value);
#elif defined(POLAR_CC_MSVC)
      unsigned long index;
      _bitscanForward(&index, value);
      return index;
#endif
   }
};

#if !defined(POLAR_CC_MSVC) || defined(_M_X64)
template <typename T>
struct TrailingZerosCounter<T, 8>
{
   static std::size_t count(T value, ZeroBehavior zb) {
      if (zb != ZeroBehavior::Undefined && value == 0) {
         return 64;
      }
#if __has_builtin(__builtin_ctzll) || POLAR_GNUC_PREREQ(4, 0, 0)
      return __builtin_ctzll(value);
#elif defined(_MSC_VER)
      unsigned long index;
      _bitscanForward64(&index, value);
      return index;
#endif
   }
};
#endif
#endif

} // internal

/// \brief Count number of 0's from the least significant bit to the most
///   stopping at the first 1.
///
/// Only unsigned integral types are allowed.
///
/// \param ZB the behavior on an input of 0. Only ZeroBehavior::Width and ZeroBehavior::Undefined are
///   valid arguments.
template <typename T>
std::size_t count_trailing_zeros(T value, ZeroBehavior zb = ZeroBehavior::Width)
{
   static_assert(std::numeric_limits<T>::is_integer &&
                 !std::numeric_limits<T>::is_signed,
                 "Only unsigned integral types are allowed.");
   return internal::TrailingZerosCounter<T, sizeof(T)>::count(value, value);
}

namespace internal {

template <typename T, std::size_t SizeOfType>
struct LeadingZerosCounter {
   static std::size_t count(T value, ZeroBehavior) {
      if (!value) {
         return std::numeric_limits<T>::digits;
      }
      // Bisection method.
      std::size_t zerobits = 0;
      for (T shift = std::numeric_limits<T>::digits >> 1; shift; shift >>= 1) {
         T temp = value >> shift;
         if (temp) {
            value = temp;
         } else {
            zerobits |= shift;
         }
      }
      return zerobits;
   }
};

#if __GNUC__ >= 4 || defined(POLAR_CC_MSVC)
template <typename T> struct LeadingZerosCounter<T, 4> {
   static std::size_t count(T value, ZeroBehavior zb) {
      if (zb != ZeroBehavior::Undefined && value == 0) {
         return 32;
      }
#if __has_builtin(__builtin_clz) || POLAR_GNUC_PREREQ(4, 0, 0)
      return __builtin_clz(value);
#elif defined(POLAR_CC_MSVC)
      unsigned long index;
      _bitscanReverse(&index, value);
      return index ^ 31;
#endif
   }
};

#if !defined(POLAR_CC_MSVC) || defined(_M_X64)
template <typename T> struct LeadingZerosCounter<T, 8> {
   static std::size_t count(T value, ZeroBehavior zb) {
      if (zb != ZeroBehavior::Undefined && value == 0) {
         return 64;
      }
#if __has_builtin(__builtin_clzll) || POLAR_GNUC_PREREQ(4, 0, 0)
      return __builtin_clzll(value);
#elif defined(POLAR_CC_MSVC)
      unsigned long index;
      _bitscanReverse64(&index, value);
      return index ^ 63;
#endif
   }
};
#endif
#endif

} // internal

/// \brief Count number of 0's from the most significant bit to the least
///   stopping at the first 1.
///
/// Only unsigned integral types are allowed.
///
/// \param ZB the behavior on an input of 0. Only ZeroBehavior::Width and ZeroBehavior::Undefined are
///   valid arguments.
template <typename T>
std::size_t count_leading_zeros(T value, ZeroBehavior zb = ZeroBehavior::Width)
{
   static_assert(std::numeric_limits<T>::is_integer &&
                 !std::numeric_limits<T>::is_signed,
                 "Only unsigned integral types are allowed.");
   return internal::LeadingZerosCounter<T, sizeof(T)>::count(value, zb);
}

/// \brief Get the index of the first set bit starting from the least
///   significant bit.
///
/// Only unsigned integral types are allowed.
///
/// \param ZB the behavior on an input of 0. Only ZeroBehavior::Max and ZeroBehavior::Undefined are
///   valid arguments.
template <typename T>
T find_first_set(T value, ZeroBehavior zb = ZeroBehavior::Max)
{
   if (zb == ZeroBehavior::Max && value == 0) {
      return std::numeric_limits<T>::max();
   }
   return count_trailing_zeros(value, ZeroBehavior::Undefined);
}

/// \brief Create a bitmask with the N right-most bits set to 1, and all other
/// bits set to 0.  Only unsigned types are allowed.
template <typename T>
T mask_trailing_ones(unsigned size)
{
   static_assert(std::is_unsigned<T>::value, "Invalid type!");
   const unsigned bits = CHAR_BIT * sizeof(T);
   assert(size <= bits && "Invalid bit index");
   return size == 0 ? 0 : (T(-1) >> (bits - size));
}

/// \brief Create a bitmask with the N left-most bits set to 1, and all other
/// bits set to 0.  Only unsigned types are allowed.
template <typename T>
T mask_leading_ones(unsigned size)
{
   return ~mask_trailing_ones<T>(CHAR_BIT * sizeof(T) - size);
}

/// \brief Create a bitmask with the N right-most bits set to 0, and all other
/// bits set to 1.  Only unsigned types are allowed.
template <typename T>
T mask_trailing_zeros(unsigned size)
{
   return mask_leading_ones<T>(CHAR_BIT * sizeof(T) - size);
}

/// \brief Create a bitmask with the N left-most bits set to 0, and all other
/// bits set to 1.  Only unsigned types are allowed.
template <typename T>
T mask_leading_zeros(unsigned size)
{
   return mask_trailing_ones<T>(CHAR_BIT * sizeof(T) - size);
}

/// \brief Get the index of the last set bit starting from the least
///   significant bit.
///
/// Only unsigned integral types are allowed.
///
/// \param ZB the behavior on an input of 0. Only ZeroBehavior::Max and ZeroBehavior::Undefined are
///   valid arguments.
template <typename T>
T find_last_set(T value, ZeroBehavior zb = ZeroBehavior::Max)
{
   if (zb == ZeroBehavior::Max && value == 0) {
      return std::numeric_limits<T>::max();
   }
   // Use ^ instead of - because both gcc and llvm can remove the associated ^
   // in the __builtin_clz intrinsic on x86.
   return count_leading_zeros(value, ZeroBehavior::Undefined) ^
         (std::numeric_limits<T>::digits - 1);
}

/// \brief Macro compressed bit reversal table for 256 bits.
///
/// http://graphics.stanford.edu/~seander/bithacks.html#BitReverseTable
static const unsigned char sg_bitReverseTable256[256] =
{
   #define R2(n) n, n + 2 * 64, n + 1 * 64, n + 3 * 64
   #define R4(n) R2(n), R2(n + 2 * 16), R2(n + 1 * 16), R2(n + 3 * 16)
   #define R6(n) R4(n), R4(n + 2 * 4), R4(n + 1 * 4), R4(n + 3 * 4)
   R6(0), R6(2), R6(1), R6(3)
   #undef R2
   #undef R4
   #undef R6
};

/// \brief Reverse the bits in \p Val.
template <typename T>
T reverse_bits(T value)
{
   unsigned char in[sizeof(value)];
   unsigned char out[sizeof(value)];
   std::memcpy(in, &value, sizeof(value));
   for (unsigned i = 0; i < sizeof(value); ++i) {
      out[(sizeof(value) - i) - 1] = sg_bitReverseTable256[in[i]];
   }
   std::memcpy(&value, out, sizeof(value));
   return value;
}

// NOTE: The following support functions use the _32/_64 extensions instead of
// type overloading so that signed and unsigned integers can be used without
// ambiguity.

/// Return the high 32 bits of a 64 bit value.
constexpr inline uint32_t high_32(uint64_t value)
{
   return static_cast<uint32_t>(value >> 32);
}

/// Return the low 32 bits of a 64 bit value.
constexpr inline uint32_t low_32(uint64_t value)
{
   return static_cast<uint32_t>(value);
}

/// Make a 64-bit integer from a high / low pair of 32-bit integers.
constexpr inline uint64_t make_64(uint32_t high, uint32_t low)
{
   return ((uint64_t)high << 32) | (uint64_t)low;
}

/// Checks if an integer fits into the given bit width.
template <unsigned N>
constexpr inline bool is_int(int64_t value)
{
   return N >= 64 || (-(INT64_C(1)<<(N-1)) <= value && value < (INT64_C(1)<<(N-1)));
}
// Template specializations to get better code for common cases.
template <> constexpr inline bool is_int<8>(int64_t value)
{
   return static_cast<int8_t>(value) == value;
}

template <> constexpr inline bool is_int<16>(int64_t value)
{
   return static_cast<int16_t>(value) == value;
}

template <> constexpr inline bool is_int<32>(int64_t value)
{
   return static_cast<int32_t>(value) == value;
}

/// Checks if a signed integer is an N bit number shifted left by S.
template <unsigned N, unsigned S>
constexpr inline bool is_shifted_int(int64_t value)
{
   static_assert(
            N > 0, "is_shifted_int<0> doesn't make sense (refers to a 0-bit number.");
   static_assert(N + S <= 64, "is_shifted_int<N, S> with N + S > 64 is too wide.");
   return is_int<N + S>(value) && (value % (UINT64_C(1) << S) == 0);
}

/// Checks if an unsigned integer fits into the given bit width.
///
/// This is written as two functions rather than as simply
///
///   return N >= 64 || value < (UINT64_C(1) << N);
///
/// to keep MSVC from (incorrectly) warning on is_uint<64> that we're shifting
/// left too many places.
template <unsigned N>
constexpr inline typename std::enable_if<(N < 64), bool>::type
is_uint(uint64_t value)
{
   static_assert(N > 0, "is_uint<0> doesn't make sense");
   return value < (UINT64_C(1) << (N));
}

template <unsigned N>
constexpr inline typename std::enable_if<N >= 64, bool>::type
is_uint(uint64_t value)
{
   return true;
}

// Template specializations to get better code for common cases.
template <> constexpr inline bool is_uint<8>(uint64_t value)
{
   return static_cast<uint8_t>(value) == value;
}

template <> constexpr inline bool is_uint<16>(uint64_t value)
{
   return static_cast<uint16_t>(value) == value;
}

template <> constexpr inline bool is_uint<32>(uint64_t value)
{
   return static_cast<uint32_t>(value) == value;
}

/// Checks if a unsigned integer is an N bit number shifted left by S.
template <unsigned N, unsigned S>
constexpr inline bool is_shifted_uint(uint64_t value)
{
   static_assert(
            N > 0, "is_shifted_uint<0> doesn't make sense (refers to a 0-bit number)");
   static_assert(N + S <= 64,
                 "is_shifted_uint<N, S> with N + S > 64 is too wide.");
   // Per the two static_asserts above, S must be strictly less than 64.  So
   // 1 << S is not undefined behavior.
   return is_uint<N + S>(value) && (value % (UINT64_C(1) << S) == 0);
}

/// Gets the maximum value for a N-bit unsigned integer.
inline uint64_t max_uint_n(uint64_t size)
{
   assert(size > 0 && size <= 64 && "integer width out of range");
   // uint64_t(1) << 64 is undefined behavior, so we can't do
   //   (uint64_t(1) << N) - 1
   // without checking first that N != 64.  But this works and doesn't have a
   // branch.
   return UINT64_MAX >> (64 - size);
}

/// Gets the minimum value for a N-bit signed integer.
inline int64_t min_int_n(int64_t size)
{
   assert(size > 0 && size <= 64 && "integer width out of range");
   return -(UINT64_C(1)<<(size-1));
}

/// Gets the maximum value for a N-bit signed integer.
inline int64_t max_int_n(int64_t size)
{
   assert(size > 0 && size <= 64 && "integer width out of range");
   // This relies on two's complement wraparound when N == 64, so we convert to
   // int64_t only at the very end to avoid UB.
   return (UINT64_C(1) << (size - 1)) - 1;
}

/// Checks if an unsigned integer fits into the given (dynamic) bit width.
inline bool is_uint_n(unsigned size, uint64_t value)
{
   return size >= 64 || value <= max_uint_n(size);
}

/// Checks if an signed integer fits into the given (dynamic) bit width.
inline bool is_int_n(unsigned size, int64_t value)
{
   return size >= 64 || (min_int_n(size) <= value && value <= max_int_n(size));
}

/// Return true if the argument is a non-empty sequence of ones starting at the
/// least significant bit with the remainder zero (32 bit version).
/// Ex. is_mask(0x0000FFFFU) == true.
constexpr inline bool is_mask(uint32_t value)
{
   return value && ((value + 1) & value) == 0;
}

/// Return true if the argument is a non-empty sequence of ones starting at the
/// least significant bit with the remainder zero (64 bit version).
constexpr inline bool is_mask(uint64_t value)
{
   return value && ((value + 1) & value) == 0;
}

/// Return true if the argument contains a non-empty sequence of ones with the
/// remainder zero (32 bit version.) Ex. is_shifted_mask(0x0000FF00U) == true.
constexpr inline bool is_shifted_mask(uint32_t value)
{
   return value && is_mask((value - 1) | value);
}

/// Return true if the argument contains a non-empty sequence of ones with the
/// remainder zero (64 bit version.)
constexpr inline bool is_shifted_mask(uint64_t value)
{
   return value && is_mask((value - 1) | value);
}

/// Return true if the argument is a power of two > 0.
/// Ex. is_power_of_two(0x00100000U) == true (32 bit edition.)
constexpr inline bool is_power_of_two(uint32_t value)
{
   return value && !(value & (value - 1));
}

/// Return true if the argument is a power of two > 0 (64 bit edition.)
constexpr inline bool is_power_of_two(uint64_t value)
{
   return value && !(value & (value - 1));
}

/// Return a byte-swapped representation of the 16-bit argument.
inline uint16_t byte_swap(uint16_t value)
{
   return polar::utils::swap_byte_order_16(value);
}

/// Return a byte-swapped representation of the 32-bit argument.
inline uint32_t byte_swap(uint32_t value)
{
   return polar::utils::swap_byte_order_32(value);
}

/// Return a byte-swapped representation of the 64-bit argument.
inline uint64_t byte_swap(uint64_t value)
{
   return polar::utils::swap_byte_order_64(value);
}

/// \brief Count the number of ones from the most significant bit to the first
/// zero bit.
///
/// Ex. countLeadingOnes(0xFF0FFF00) == 8.
/// Only unsigned integral types are allowed.
///
/// \param zb the behavior on an input of all ones. Only ZeroBehavior::Width and
/// ZeroBehavior::Undefined are valid arguments.
template <typename T>
std::size_t count_leading_ones(T value, ZeroBehavior zb = ZeroBehavior::Width)
{
   static_assert(std::numeric_limits<T>::is_integer &&
                 !std::numeric_limits<T>::is_signed,
                 "Only unsigned integral types are allowed.");
   return count_leading_zeros(~value, zb);
}

/// \brief Count the number of ones from the least significant bit to the first
/// zero bit.
///
/// Ex. count_trailing_ones(0x00FF00FF) == 8.
/// Only unsigned integral types are allowed.
///
/// \param zb the behavior on an input of all ones. Only ZeroBehavior::Width and
/// ZeroBehavior::Undefined are valid arguments.
template <typename T>
std::size_t count_trailing_ones(T value, ZeroBehavior zb = ZeroBehavior::Width)
{
   static_assert(std::numeric_limits<T>::is_integer &&
                 !std::numeric_limits<T>::is_signed,
                 "Only unsigned integral types are allowed.");
   return count_trailing_zeros(~value, zb);
}

namespace internal {

template <typename T, std::size_t SizeOfT>
struct PopulationCounter
{
   static unsigned count(T value)
   {
      // Generic version, forward to 32 bits.
      static_assert(SizeOfT <= 4, "Not implemented!");
#if __GNUC__ >= 4
      return __builtin_popcount(value);
#else
      uint32_t v = value;
      v = v - ((v >> 1) & 0x55555555);
      v = (v & 0x33333333) + ((v >> 2) & 0x33333333);
      return ((v + (v >> 4) & 0xF0F0F0F) * 0x1010101) >> 24;
#endif
   }
};

template <typename T>
struct PopulationCounter<T, 8>
{
   static unsigned count(T value)
   {
#if __GNUC__ >= 4
      return __builtin_popcountll(value);
#else
      uint64_t v = value;
      v = v - ((v >> 1) & 0x5555555555555555ULL);
      v = (v & 0x3333333333333333ULL) + ((v >> 2) & 0x3333333333333333ULL);
      v = (v + (v >> 4)) & 0x0F0F0F0F0F0F0F0FULL;
      return unsigned((uint64_t)(v * 0x0101010101010101ULL) >> 56);
#endif
   }
};

} // internal

/// \brief Count the number of set bits in a value.
/// Ex. countPopulation(0xF000F000) = 8
/// Returns 0 if the word is zero.
template <typename T>
inline unsigned count_population(T value)
{
   static_assert(std::numeric_limits<T>::is_integer &&
                 !std::numeric_limits<T>::is_signed,
                 "Only unsigned integral types are allowed.");
   return internal::PopulationCounter<T, sizeof(T)>::count(value);
}

/// Return the log base 2 of the specified value.
inline double log2(double value)
{
#if defined(__ANDROID_API__) && __ANDROID_API__ < 18
   return __builtin_log(value) / __builtin_log(2.0);
#else
   return log2(value);
#endif
}

/// Return the floor log base 2 of the specified value, -1 if the value is zero.
/// (32 bit edition.)
/// Ex. log2(32) == 5, log2(1) == 0, log2(0) == -1, log2(6) == 2
inline unsigned log2(uint32_t value)
{
   return 31 - count_leading_zeros(value);
}

/// Return the floor log base 2 of the specified value, -1 if the value is zero.
/// (64 bit edition.)
inline unsigned log2(uint64_t value)
{
   return 63 - count_leading_zeros(value);
}

/// Return the ceil log base 2 of the specified value, 32 if the value is zero.
/// (32 bit edition).
/// Ex. log2_ceil(32) == 5, log2_ceil(1) == 0, log2_ceil(6) == 3
inline unsigned log2_ceil(uint32_t value)
{
   return 32 - count_leading_zeros(value - 1);
}

/// Return the ceil log base 2 of the specified value, 64 if the value is zero.
/// (64 bit edition.)
inline unsigned log2_ceil(uint64_t value)
{
   return 64 - count_leading_zeros(value - 1);
}

/// Return the greatest common divisor of the values using Euclid's algorithm.
inline uint64_t greatest_common_divisor64(uint64_t lhs, uint64_t rhs)
{
   while (rhs) {
      uint64_t temp = rhs;
      rhs = lhs % rhs;
      lhs = temp;
   }
   return lhs;
}

/// This function takes a 64-bit integer and returns the bit equivalent double.
inline double bits_to_double(uint64_t bits)
{
   double ret;
   static_assert(sizeof(uint64_t) == sizeof(double), "Unexpected type sizes");
   memcpy(&ret, &bits, sizeof(bits));
   return ret;
}

/// This function takes a 32-bit integer and returns the bit equivalent float.
inline float bits_to_float(uint32_t bits)
{
   float ret;
   static_assert(sizeof(uint32_t) == sizeof(float), "Unexpected type sizes");
   memcpy(&ret, &bits, sizeof(bits));
   return ret;
}

/// This function takes a double and returns the bit equivalent 64-bit integer.
/// Note that copying doubles around changes the bits of NaNs on some hosts,
/// notably x86, so this routine cannot be used if these bits are needed.
inline uint64_t double_to_bits(double value)
{
   uint64_t bits;
   static_assert(sizeof(uint64_t) == sizeof(double), "Unexpected type sizes");
   memcpy(&bits, &value, sizeof(value));
   return bits;
}

/// This function takes a float and returns the bit equivalent 32-bit integer.
/// Note that copying floats around changes the bits of NaNs on some hosts,
/// notably x86, so this routine cannot be used if these bits are needed.
inline uint32_t float_to_bits(float value) {
   uint32_t bits;
   static_assert(sizeof(uint32_t) == sizeof(float), "Unexpected type sizes");
   memcpy(&bits, &value, sizeof(value));
   return bits;
}

/// A and B are either alignments or offsets. Return the minimum alignment that
/// may be assumed after adding the two together.
constexpr inline uint64_t min_align(uint64_t lhs, uint64_t rhs)
{
   // The largest power of 2 that divides both A and B.
   //
   // Replace "-Value" by "1+~Value" in the following commented code to avoid
   // MSVC warning C4146
   //    return (lhs | rhs) & -(lhs | rhs);
   return (lhs | rhs) & (1 + ~(lhs | rhs));
}

/// \brief Aligns \c Addr to \c Alignment bytes, rounding up.
///
/// Alignment should be a power of two.  This method rounds up, so
/// align_addr(7, 4) == 8 and align_addr(8, 4) == 8.
inline uintptr_t align_addr(const void *addr, size_t alignment)
{
   assert(alignment && is_power_of_two((uint64_t)alignment) &&
          "alignment is not a power of two!");
   assert((uintptr_t)addr + alignment - 1 >= (uintptr_t)addr);
   return (((uintptr_t)addr + alignment - 1) & ~(uintptr_t)(alignment - 1));
}

/// \brief Returns the necessary adjustment for aligning \c Ptr to \c Alignment
/// bytes, rounding up.
inline size_t alignment_adjustment(const void *ptr, size_t alignment)
{
   return align_addr(ptr, alignment) - (uintptr_t)ptr;
}

/// Returns the next power of two (in 64-bits) that is strictly greater than A.
/// Returns zero on overflow.
inline uint64_t next_power_of_two(uint64_t value)
{
   value |= (value >> 1);
   value |= (value >> 2);
   value |= (value >> 4);
   value |= (value >> 8);
   value |= (value >> 16);
   value |= (value >> 32);
   return value + 1;
}

/// Returns the power of two which is less than or equal to the given value.
/// Essentially, it is a floor operation across the domain of powers of two.
inline uint64_t power_of_two_floor(uint64_t value)
{
   if (!value) {
      return 0;
   }
   return 1ull << (63 - count_leading_zeros(value, ZeroBehavior::Undefined));
}

/// Returns the power of two which is greater than or equal to the given value.
/// Essentially, it is a ceil operation across the domain of powers of two.
inline uint64_t power_of_two_ceil(uint64_t value)
{
   if (!value) {
      return 0;
   }
   return next_power_of_two(value - 1);
}

/// Returns the next integer (mod 2**64) that is greater than or equal to
/// \p Value and is a multiple of \p Align. \p Align must be non-zero.
///
/// If non-zero \p Skew is specified, the return value will be a minimal
/// integer that is greater than or equal to \p Value and equal to
/// \p Align * N + \p Skew for some integer N. If \p Skew is larger than
/// \p Align, its value is adjusted to '\p Skew mod \p Align'.
///
/// Examples:
/// \code
///   align_to(5, 8) = 8
///   align_to(17, 8) = 24
///   align_to(~0LL, 8) = 0
///   align_to(321, 255) = 510
///
///   align_to(5, 8, 7) = 7
///   align_to(17, 8, 1) = 17
///   align_to(~0LL, 8, 3) = 3
///   align_to(321, 255, 42) = 552
/// \endcode
inline uint64_t align_to(uint64_t value, uint64_t align, uint64_t skew = 0)
{
   assert(align != 0u && "align can't be 0.");
   skew %= align;
   return (value + align - 1 - skew) / align * align + skew;
}

/// Returns the next integer (mod 2**64) that is greater than or equal to
/// \p Value and is a multiple of \c Align. \c Align must be non-zero.
template <uint64_t Align>
constexpr inline uint64_t align_to(uint64_t value)
{
   static_assert(Align != 0u, "Align must be non-zero");
   return (value + Align - 1) / Align * Align;
}

/// Returns the integer ceil(Numerator / Denominator).
inline uint64_t divide_ceil(uint64_t numerator, uint64_t denominator)
{
   return align_to(numerator, denominator) / denominator;
}

/// \c align_to for contexts where a constant expression is required.
/// \sa align_to
///
/// \todo FIXME: remove when \c constexpr becomes really \c constexpr
template <uint64_t Align>
struct AlignTo
{
   static_assert(Align != 0u, "Align must be non-zero");
   template <uint64_t Value>
   struct FromValue {
      static const uint64_t value = (Value + Align - 1) / Align * Align;
   };
};

/// Returns the largest uint64_t less than or equal to \p Value and is
/// \p Skew mod \p Align. \p Align must be non-zero
inline uint64_t align_down(uint64_t value, uint64_t align, uint64_t skew = 0)
{
   assert(align != 0u && "Align can't be 0.");
   skew %= align;
   return (value - skew) / align * align + skew;
}

/// Returns the offset to the next integer (mod 2**64) that is greater than
/// or equal to \p Value and is a multiple of \p Align. \p Align must be
/// non-zero.
inline uint64_t offset_to_alignment(uint64_t value, uint64_t align)
{
   return align_to(value, align) - value;
}

/// Sign-extend the number in the bottom B bits of X to a 32-bit integer.
/// Requires 0 < B <= 32.
template <unsigned BitWidth>
constexpr inline int32_t sign_extend(uint32_t value)
{
   static_assert(BitWidth > 0, "Bit width can't be 0.");
   static_assert(BitWidth <= 32, "Bit width out of range.");
   return int32_t(value << (32 - BitWidth)) >> (32 - BitWidth);
}

/// Sign-extend the number in the bottom B bits of X to a 32-bit integer.
/// Requires 0 < B < 32.
inline int32_t sign_extend(uint32_t value, unsigned bitWidth)
{
   assert(bitWidth > 0 && "Bit width can't be 0.");
   assert(bitWidth <= 32 && "Bit width out of range.");
   return int32_t(value << (32 - bitWidth)) >> (32 - bitWidth);
}

/// Sign-extend the number in the bottom B bits of X to a 64-bit integer.
/// Requires 0 < B < 64.
template <unsigned BitWidth>
constexpr inline int64_t sign_extend(uint64_t value)
{
   static_assert(BitWidth > 0, "Bit width can't be 0.");
   static_assert(BitWidth <= 64, "Bit width out of range.");
   return int64_t(value << (64 - BitWidth)) >> (64 - BitWidth);
}

/// Sign-extend the number in the bottom B bits of X to a 64-bit integer.
/// Requires 0 < B < 64.
inline int64_t sign_extend(uint64_t value, unsigned bitWidth)
{
   assert(bitWidth > 0 && "Bit width can't be 0.");
   assert(bitWidth <= 64 && "Bit width out of range.");
   return int64_t(value << (64 - bitWidth)) >> (64 - bitWidth);
}

/// Subtract two unsigned integers, X and Y, of type T and return the absolute
/// value of the result.
template <typename T>
typename std::enable_if<std::is_unsigned<T>::value, T>::type
absolute_difference(T lhs, T rhs)
{
   return std::max(lhs, rhs) - std::min(lhs, rhs);
}

/// Add two unsigned integers, X and Y, of type T.  Clamp the result to the
/// maximum representable value of T on overflow.  resultOverflowed indicates if
/// the result is larger than the maximum representable value of type T.
template <typename T>
typename std::enable_if<std::is_unsigned<T>::value, T>::type
saturating_add(T lhs, T rhs, bool *resultOverflowed = nullptr)
{
   bool dummy;
   bool &overflowed = resultOverflowed ? *resultOverflowed : dummy;
   // Hacker's Delight, p. 29
   T ret = lhs + rhs;
   overflowed = (ret < lhs || ret < rhs);
   if (overflowed) {
      return std::numeric_limits<T>::max();
   } else {
      return ret;
   }
}

/// Multiply two unsigned integers, X and Y, of type T.  Clamp the result to the
/// maximum representable value of T on overflow.  resultOverflowed indicates if
/// the result is larger than the maximum representable value of type T.
template <typename T>
typename std::enable_if<std::is_unsigned<T>::value, T>::type
saturating_multiply(T lhs, T rhs, bool *resultOverflowed = nullptr)
{
   bool dummy;
   bool &overflowed = resultOverflowed ? *resultOverflowed : dummy;

   // Hacker's Delight, p. 30 has a different algorithm, but we don't use that
   // because it fails for uint16_t (where multiplication can have undefined
   // behavior due to promotion to int), and requires a division in addition
   // to the multiplication.

   overflowed = false;

   // log2(Z) would be either log2Z or log2Z + 1.
   // Special case: if X or Y is 0, log2 gives -1, and log2Z
   // will necessarily be less than log2Max as desired.
   int log2Z = log2(lhs) + log2(rhs);
   const T max = std::numeric_limits<T>::max();
   int log2Max = log2(max);
   if (log2Z < log2Max) {
      return lhs * rhs;
   }
   if (log2Z > log2Max) {
      overflowed = true;
      return max;
   }

   // We're going to use the top bit, and maybe overflow one
   // bit past it. Multiply all but the bottom bit then add
   // that on at the end.
   T ret = (lhs >> 1) * rhs;
   if (ret & ~(max >> 1)) {
      overflowed = true;
      return max;
   }
   ret <<= 1;
   if (lhs & 1) {
      return saturating_add(ret, rhs, resultOverflowed);
   }
   return ret;
}

/// Multiply two unsigned integers, X and Y, and add the unsigned integer, A to
/// the product. Clamp the result to the maximum representable value of T on
/// overflow. resultOverflowed indicates if the result is larger than the
/// maximum representable value of type T.
template <typename T>
typename std::enable_if<std::is_unsigned<T>::value, T>::type
saturating_multiply_add(T lhs, T rhs, T sum, bool *resultOverflowed = nullptr)
{
   bool dummy;
   bool &overflowed = resultOverflowed ? *resultOverflowed : dummy;
   T product = saturating_multiply(lhs, rhs, &overflowed);
   if (overflowed) {
      return product;
   }
   return saturating_add(sum, product, &overflowed);
}

/// Use this rather than HUGE_VALF; the latter causes warnings on MSVC.
extern const float g_hugeValf;

} // utils
} // polar

#endif // POLAR_UTILS_MATH_EXTRAS_H
