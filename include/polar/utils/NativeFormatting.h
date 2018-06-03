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

#ifndef POLAR_SUPPORT_NATIVE_FORMATTING_H
#define POLAR_SUPPORT_NATIVE_FORMATTING_H

#include "polar/utils/RawOutStream.h"
#include <cstdint>
#include <optional>

namespace polar {
namespace utils {

enum class FloatStyle
{
   Exponent,
   ExponentUpper,
   Fixed,
   Percent
};

enum class IntegerStyle
{
   Integer,
   Number,
};

enum class HexPrintStyle
{
   Upper,
   Lower,
   PrefixUpper,
   PrefixLower
};

size_t get_default_precision(FloatStyle style);

bool is_prefixed_hex_style(HexPrintStyle style);

void write_integer(RawOutStream &outStream, unsigned int size, size_t minDigits,
                   IntegerStyle style);
void write_integer(RawOutStream &outStream, int size, size_t minDigits, IntegerStyle Style);
void write_integer(RawOutStream &outStream, unsigned long size, size_t minDigits,
                   IntegerStyle style);
void write_integer(RawOutStream &outStream, long size, size_t minDigits,
                   IntegerStyle style);
void write_integer(RawOutStream &outStream, unsigned long long size, size_t minDigits,
                   IntegerStyle style);
void write_integer(RawOutStream &outStream, long long size, size_t minDigits,
                   IntegerStyle style);

void write_hex(RawOutStream &outStream, uint64_t size, HexPrintStyle style,
               std::optional<size_t> width = std::nullopt);
void write_double(RawOutStream &outStream, double D, FloatStyle style,
                  std::optional<size_t> precision = std::nullopt);

} // utils
} // polar

#endif // POLAR_SUPPORT_NATIVE_FORMATTING_H
