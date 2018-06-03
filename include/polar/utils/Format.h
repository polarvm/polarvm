// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/06/03.

//===----------------------------------------------------------------------===//
//
// This file implements the format() function, which can be used with other
// LLVM subsystems to provide printf-style formatting.  This gives all the power
// and risk of printf.  This can be used like this (with RawOstreams as an
// example):
//
//    outStream << "mynumber: " << format("%4.5f", 1234.412) << '\n';
//
// Or if you prefer:
//
//  outStream << format("mynumber: %4.5f\n", 1234.412);
//
//===----------------------------------------------------------------------===//

#ifndef POLAR_UTILS_FORMAT_H
#define POLAR_UTILS_FORMAT_H

#include "polar/basic/adt/ArrayRef.h"
#include "polar/basic/adt/StlExtras.h"
#include "polar/basic/adt/StringRef.h"
#include "polar/global/DataTypes.h"
#include <cassert>
#include <cstdio>
#include <tuple>

namespace polar {
namespace utils {

using polar::basic::ArrayRef;

/// This is a helper class used for handling formatted output.  It is the
/// abstract base class of a templated derived class.
class FormatObjectBase
{
protected:
   const char *m_fmt;
   ~FormatObjectBase() = default; // Disallow polymorphic deletion.
   FormatObjectBase(const FormatObjectBase &) = default;
   virtual void home(); // Out of line virtual method.

   /// Call snprintf() for this object, on the given buffer and size.
   virtual int snprint(char *buffer, unsigned bufferSize) const = 0;

public:
   FormatObjectBase(const char *fmt)
      : m_fmt(fmt)
   {}

   /// Format the object into the specified buffer.  On success, this returns
   /// the length of the formatted string.  If the buffer is too small, this
   /// returns a length to retry with, which will be larger than BufferSize.
   unsigned print(char *buffer, unsigned bufferSize) const
   {
      assert(bufferSize && "Invalid buffer size!");

      // Print the string, leaving room for the terminating null.
      int size = snprint(buffer, bufferSize);

      // VC++ and old GlibC return negative on overflow, just double the size.
      if (size < 0) {
         return bufferSize * 2;
      }
      // Other implementations yield number of bytes needed, not including the
      // final '\0'.
      if (unsigned(size) >= bufferSize) {
         return size + 1;
      }
      // Otherwise N is the length of output (not including the final '\0').
      return size;
   }
};

/// These are templated helper classes used by the format function that
/// capture the object to be formatted and the format string. When actually
/// printed, this synthesizes the string into a temporary buffer provided and
/// returns whether or not it is big enough.

// Helper to validate that format() parameters are scalars or pointers.
template <typename... Args>
struct ValidateFormatParameters;
template <typename Arg, typename... Args>
struct ValidateFormatParameters<Arg, Args...>
{
   static_assert(std::is_scalar<Arg>::value,
                 "format can't be used with non fundamental / non pointer type");
   ValidateFormatParameters()
   {
      ValidateFormatParameters<Args...>();
   }
};

template <>
struct ValidateFormatParameters<>
{};

template <typename... Ts>
class FormatObject final : public FormatObjectBase
{
   std::tuple<Ts...> m_values;

   template <std::size_t... Is>
   int snprintTuple(char *buffer, unsigned bufferSize,
                    polar::basic::index_sequence<Is...>) const
   {
#ifdef _MSC_VER
      return _snprintf(buffer, bufferSize, m_fmt, std::get<Is>(m_values)...);
#else
      return snprintf(buffer, bufferSize, m_fmt, std::get<Is>(m_values)...);
#endif
   }

public:
   FormatObject(const char *fmt, const Ts &... values)
      : FormatObjectBase(fmt), m_values(values...)
   {
      ValidateFormatParameters<Ts...>();
   }

   int snprint(char *buffer, unsigned bufferSize) const override
   {
      return snprint_tuple(buffer, bufferSize, polar::basic::index_sequence_for<Ts...>());
   }
};

/// These are helper functions used to produce formatted output.  They use
/// template type deduction to construct the appropriate instance of the
/// FormatObject class to simplify their construction.
///
/// This is typically used like:
/// \code
///   OS << format("%0.4f", myfloat) << '\n';
/// \endcode

template <typename... Ts>
inline FormatObject<Ts...> format(const char *fmt, const Ts &... values)
{
   return FormatObject<Ts...>(fmt, values...);
}

/// This is a helper class for left_justify, right_justify, and center_justify.
class FormattedString
{
public:
   enum Justification
   {
      JustifyNone,
      JustifyLeft,
      JustifyRight,
      JustifyCenter
   };
   FormattedString(StringRef str, unsigned width, Justification justification)
      : m_str(str), m_width(width), m_justify(justification) {}

private:
   StringRef m_str;
   unsigned m_width;
   Justification m_justify;
   friend class RawOutStream;
};

/// left_justify - append spaces after string so total output is
/// \p Width characters.  If \p Str is larger that \p Width, full string
/// is written with no padding.
inline FormattedString left_justify(StringRef str, unsigned width)
{
   return FormattedString(str, width, FormattedString::JustifyLeft);
}

/// right_justify - add spaces before string so total output is
/// \p Width characters.  If \p Str is larger that \p Width, full string
/// is written with no padding.
inline FormattedString right_justify(StringRef str, unsigned width)
{
   return FormattedString(str, width, FormattedString::JustifyRight);
}

/// center_justify - add spaces before and after string so total output is
/// \p Width characters.  If \p Str is larger that \p Width, full string
/// is written with no padding.
inline FormattedString center_justify(StringRef str, unsigned width)
{
   return FormattedString(str, width, FormattedString::JustifyCenter);
}

/// This is a helper class used for format_hex() and format_decimal().
class FormattedNumber
{
   uint64_t m_hexValue;
   int64_t m_decValue;
   unsigned m_width;
   bool m_hex;
   bool m_upper;
   bool m_hexPrefix;
   friend class RawOutStream;

public:
   FormattedNumber(uint64_t hexValue, int64_t decValue, unsigned width, bool isHex, bool isUpper,
                   bool isPrefix)
      : m_hexValue(HV),
        m_decValue(decValue),
        m_width(width),
        m_hex(isHex),
        m_upper(isUpper),
        HexPrefix(Prefix)
   {}
};

/// format_hex - Output \p N as a fixed width hexadecimal. If number will not
/// fit in width, full number is still printed.  Examples:
///   OS << format_hex(255, 4)              => 0xff
///   OS << format_hex(255, 4, true)        => 0xFF
///   OS << format_hex(255, 6)              => 0x00ff
///   OS << format_hex(255, 2)              => 0xff
inline FormattedNumber format_hex(uint64_t hexValue, unsigned width,
                                  bool isUpper = false)
{
   assert(width <= 18 && "hex width must be <= 18");
   return FormattedNumber(hexValue, 0, width, true, isUpper, true);
}

/// format_hex_no_prefix - Output \p N as a fixed width hexadecimal. Does not
/// prepend '0x' to the outputted string.  If number will not fit in width,
/// full number is still printed.  Examples:
///   OS << format_hex_no_prefix(255, 2)              => ff
///   OS << format_hex_no_prefix(255, 2, true)        => FF
///   OS << format_hex_no_prefix(255, 4)              => 00ff
///   OS << format_hex_no_prefix(255, 1)              => ff
inline FormattedNumber format_hex_no_prefix(uint64_t hexValue, unsigned width,
                                            bool isUpper = false)
{
   assert(width <= 16 && "hex width must be <= 16");
   return FormattedNumber(hexValue, 0, width, true, isUpper, false);
}

/// format_decimal - Output \p N as a right justified, fixed-width decimal. If
/// number will not fit in width, full number is still printed.  Examples:
///   OS << format_decimal(0, 5)     => "    0"
///   OS << format_decimal(255, 5)   => "  255"
///   OS << format_decimal(-1, 3)    => " -1"
///   OS << format_decimal(12345, 3) => "12345"
inline FormattedNumber format_decimal(int64_t decValue, unsigned width)
{
   return FormattedNumber(0, decValue, width, false, false, false);
}

class FormattedBytes
{
   ArrayRef<uint8_t> m_bytes;

   // If not None, display offsets for each line relative to starting value.
   std::optional<uint64_t> m_firstByteOffset;
   uint32_t m_indentLevel;  // Number of characters to indent each line.
   uint32_t m_numPerLine;   // Number of bytes to show per line.
   uint8_t m_byteGroupSize; // How many hex bytes are grouped without spaces
   bool m_isUpper;            // Show offset and hex bytes as upper case.
   bool m_ascii;            // Show the ASCII bytes for the hex bytes to the right.
   friend class RawOutStream;

public:
   FormattedBytes(ArrayRef<uint8_t> bytes, uint32_t indentLevel, std::optional<uint64_t> firstByteOffset,
                  uint32_t numPerLine, uint8_t byteGroupSize, bool isUpper, bool ascii)
      : m_bytes(bytes),
        m_firstByteOffset(firstByteOffset),
        m_indentLevel(indentLevel),
        m_numPerLine(numPerLine),
        m_byteGroupSize(byteGroupSize),
        isupper(isUpper),
        m_ascii(ascii)
   {

      if (m_byteGroupSize > m_numPerLine) {
         m_byteGroupSize = m_numPerLine;
      }
   }
};

inline FormattedBytes
format_bytes(ArrayRef<uint8_t> bytes, std::optional<uint64_t> firstByteOffset = std::nullopt,
             uint32_t numPerLine = 16, uint8_t byteGroupSize = 4,
             uint32_t indentLevel = 0, bool isUpper = false)
{
   return FormattedBytes(bytes, indentLevel, firstByteOffset, numPerLine,
                         byteGroupSize, isUpper, false);
}

inline FormattedBytes
format_bytes_with_ascii(ArrayRef<uint8_t> bytes,
                        std::optional<uint64_t> firstByteOffset = std::nullopt,
                        uint32_t numPerLine = 16, uint8_t byteGroupSize = 4,
                        uint32_t indentLevel = 0, bool isUpper = false)
{
   return FormattedBytes(bytes, indentLevel, firstByteOffset, numPerLine,
                         byteGroupSize, isUpper, true);
}

} // utils
} // polar

#endif // POLAR_UTILS_FORMAT_H
