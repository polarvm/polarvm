// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/07/13.

#include "polar/basic/adt/SmallString.h"
#include "polar/utils/NativeFormatting.h"
#include "polar/utils/RawOutStream.h"
#include "gtest/gtest.h"
#include <type_traits>
#include <optional>

using namespace polar;
using namespace polar::basic;
using namespace polar::utils;

namespace {

template <typename T> std::string format_number(T N, IntegerStyle style)
{
   std::string S;
   RawStringOutStream Str(S);
   write_integer(Str, N, 0, style);
   Str.flush();
   return S;
}

std::string format_number(uint64_t N, HexPrintStyle style,
                          std::optional<size_t> Width = std::nullopt) {
   std::string S;
   RawStringOutStream Str(S);
   write_hex(Str, N, style, Width);
   Str.flush();
   return S;
}

std::string format_number(double D, FloatStyle style,
                          std::optional<size_t> Precision = std::nullopt)
{
   std::string S;
   RawStringOutStream Str(S);
   write_double(Str, D, style, Precision);
   Str.flush();
   return S;
}

// Test basic number formatting with various styles and default width and
// precision.
TEST(NativeFormatTest, testBasicIntegerTests)
{
   // Simple integers with no decimal.
   EXPECT_EQ("0", format_number(0, IntegerStyle::Integer));
   EXPECT_EQ("2425", format_number(2425, IntegerStyle::Integer));
   EXPECT_EQ("-2425", format_number(-2425, IntegerStyle::Integer));

   EXPECT_EQ("0", format_number(0LL, IntegerStyle::Integer));
   EXPECT_EQ("257257257235709",
             format_number(257257257235709LL, IntegerStyle::Integer));
   EXPECT_EQ("-257257257235709",
             format_number(-257257257235709LL, IntegerStyle::Integer));

   // Number formatting.
   EXPECT_EQ("0", format_number(0, IntegerStyle::Number));
   EXPECT_EQ("2,425", format_number(2425, IntegerStyle::Number));
   EXPECT_EQ("-2,425", format_number(-2425, IntegerStyle::Number));
   EXPECT_EQ("257,257,257,235,709",
             format_number(257257257235709LL, IntegerStyle::Number));
   EXPECT_EQ("-257,257,257,235,709",
             format_number(-257257257235709LL, IntegerStyle::Number));

   // Hex formatting.
   // lower case, prefix.
   EXPECT_EQ("0x0", format_number(0, HexPrintStyle::PrefixLower));
   EXPECT_EQ("0xbeef", format_number(0xbeefLL, HexPrintStyle::PrefixLower));
   EXPECT_EQ("0xdeadbeef",
             format_number(0xdeadbeefLL, HexPrintStyle::PrefixLower));

   // upper-case, prefix.
   EXPECT_EQ("0x0", format_number(0, HexPrintStyle::PrefixUpper));
   EXPECT_EQ("0xBEEF", format_number(0xbeefLL, HexPrintStyle::PrefixUpper));
   EXPECT_EQ("0xDEADBEEF",
             format_number(0xdeadbeefLL, HexPrintStyle::PrefixUpper));

   // lower-case, no prefix
   EXPECT_EQ("0", format_number(0, HexPrintStyle::Lower));
   EXPECT_EQ("beef", format_number(0xbeefLL, HexPrintStyle::Lower));
   EXPECT_EQ("deadbeef", format_number(0xdeadbeefLL, HexPrintStyle::Lower));

   // upper-case, no prefix.
   EXPECT_EQ("0", format_number(0, HexPrintStyle::Upper));
   EXPECT_EQ("BEEF", format_number(0xbeef, HexPrintStyle::Upper));
   EXPECT_EQ("DEADBEEF", format_number(0xdeadbeef, HexPrintStyle::Upper));
}

// Test basic floating point formatting with various styles and default width
// and precision.
TEST(NativeFormatTest, testBasicFloatingPointTests)
{
   // Double
   EXPECT_EQ("0.000000e+00", format_number(0.0, FloatStyle::Exponent));
   EXPECT_EQ("-0.000000e+00", format_number(-0.0, FloatStyle::Exponent));
   EXPECT_EQ("1.100000e+00", format_number(1.1, FloatStyle::Exponent));
   EXPECT_EQ("1.100000E+00", format_number(1.1, FloatStyle::ExponentUpper));

   // Default precision is 2 for floating points.
   EXPECT_EQ("1.10", format_number(1.1, FloatStyle::Fixed));
   EXPECT_EQ("1.34", format_number(1.34, FloatStyle::Fixed));
   EXPECT_EQ("1.34", format_number(1.344, FloatStyle::Fixed));
   EXPECT_EQ("1.35", format_number(1.346, FloatStyle::Fixed));
}

// Test common boundary cases and min/max conditions.
TEST(NativeFormatTest, testBoundaryTests)
{
   // Min and max.
   EXPECT_EQ("18446744073709551615",
             format_number(UINT64_MAX, IntegerStyle::Integer));

   EXPECT_EQ("9223372036854775807",
             format_number(INT64_MAX, IntegerStyle::Integer));
   EXPECT_EQ("-9223372036854775808",
             format_number(INT64_MIN, IntegerStyle::Integer));

   EXPECT_EQ("4294967295", format_number(UINT32_MAX, IntegerStyle::Integer));
   EXPECT_EQ("2147483647", format_number(INT32_MAX, IntegerStyle::Integer));
   EXPECT_EQ("-2147483648", format_number(INT32_MIN, IntegerStyle::Integer));

   EXPECT_EQ("nan", format_number(std::numeric_limits<double>::quiet_NaN(),
                                  FloatStyle::Fixed));
   EXPECT_EQ("INF", format_number(std::numeric_limits<double>::infinity(),
                                  FloatStyle::Fixed));
}

TEST(NativeFormatTest, testHexTests)
{
   // Test hex formatting with different widths and precisions.

   // Width less than the value should print the full value anyway.
   EXPECT_EQ("0x0", format_number(0, HexPrintStyle::PrefixLower, 0));
   EXPECT_EQ("0xabcde", format_number(0xABCDE, HexPrintStyle::PrefixLower, 3));

   // Precision greater than the value should pad with 0s.
   // TODO: The prefix should not be counted in the precision.  But unfortunately
   // it is and we have to live with it unless we fix all existing users of
   // prefixed hex formatting.
   EXPECT_EQ("0x000", format_number(0, HexPrintStyle::PrefixLower, 5));
   EXPECT_EQ("0x0abcde", format_number(0xABCDE, HexPrintStyle::PrefixLower, 8));

   EXPECT_EQ("00000", format_number(0, HexPrintStyle::Lower, 5));
   EXPECT_EQ("000abcde", format_number(0xABCDE, HexPrintStyle::Lower, 8));

   // Try printing more digits than can fit in a uint64.
   EXPECT_EQ("0x00000000000000abcde",
             format_number(0xABCDE, HexPrintStyle::PrefixLower, 21));
}

TEST(NativeFormatTest, testIntegerTests)
{
   EXPECT_EQ("-10", format_number(-10, IntegerStyle::Integer));
   EXPECT_EQ("-100", format_number(-100, IntegerStyle::Integer));
   EXPECT_EQ("-1000", format_number(-1000, IntegerStyle::Integer));
   EXPECT_EQ("-1234567890", format_number(-1234567890, IntegerStyle::Integer));
   EXPECT_EQ("10", format_number(10, IntegerStyle::Integer));
   EXPECT_EQ("100", format_number(100, IntegerStyle::Integer));
   EXPECT_EQ("1000", format_number(1000, IntegerStyle::Integer));
   EXPECT_EQ("1234567890", format_number(1234567890, IntegerStyle::Integer));
}

TEST(NativeFormatTest, testCommaTests)
{
   EXPECT_EQ("0", format_number(0, IntegerStyle::Number));
   EXPECT_EQ("10", format_number(10, IntegerStyle::Number));
   EXPECT_EQ("100", format_number(100, IntegerStyle::Number));
   EXPECT_EQ("1,000", format_number(1000, IntegerStyle::Number));
   EXPECT_EQ("1,234,567,890", format_number(1234567890, IntegerStyle::Number));

   EXPECT_EQ("-10", format_number(-10, IntegerStyle::Number));
   EXPECT_EQ("-100", format_number(-100, IntegerStyle::Number));
   EXPECT_EQ("-1,000", format_number(-1000, IntegerStyle::Number));
   EXPECT_EQ("-1,234,567,890", format_number(-1234567890, IntegerStyle::Number));
}

} // anonymous namespace
