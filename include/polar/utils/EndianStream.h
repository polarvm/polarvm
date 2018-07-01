// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/07/01.

#ifndef POLAR_UTILS_ENDIAN_STREAM_H
#define POLAR_UTILS_ENDIAN_STREAM_H

#include "polar/basic/adt/ArrayRef.h"
#include "polar/utils/Endian.h"
#include "polar/utils/RawOutStream.h"

namespace polar {
namespace utils {
namespace endian {

using polar::basic::ArrayRef;

/// Adapter to write values to a stream in a particular byte order.
template <Endianness endian> struct Writer
{
   RawOutStream &m_outstream;
   Writer(RawOutStream &outstream) : m_outstream(outstream)
   {}

   template <typename value_type> void write(ArrayRef<value_type> vals)
   {
      for (value_type value : vals) {
         write(value);
      }
   }

   template <typename value_type> void write(value_type value)
   {
      value = byte_swap<value_type, endian>(value);
      m_outstream.write((const char *)&value, sizeof(value_type));
   }
};

template <>
template <>
inline void Writer<Endianness::Little>::write<float>(float value)
{
   write(polar::utils::float_to_bits(value));
}

template <>
template <>
inline void Writer<Endianness::Little>::write<double>(double value)
{
   write(polar::utils::double_to_bits(value));
}

template <>
template <>
inline void Writer<Endianness::Big>::write<float>(float value)
{
   write(polar::utils::float_to_bits(value));
}

template <>
template <>
inline void Writer<Endianness::Big>::write<double>(double value)
{
   write(polar::utils::double_to_bits(value));
}

} // end
} // utils
} // polar

#endif // POLAR_UTILS_ENDIAN_STREAM_H
