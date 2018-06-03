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

#ifndef POLAR_UTILS_FORMAT_COMMON_H
#define POLAR_UTILS_FORMAT_COMMON_H

#include "polar/basic/adt/SmallString.h"
#include "polar/utils/FormatVariadicDetails.h"
#include "polar/utils/RawOutStream.h"

namespace polar {
namespace utils {

using polar::basic::SmallString;

enum class AlignStyle { Left, Center, Right };

struct FmtAlign
{
   internal::FormatAdapter &m_adapter;
   AlignStyle m_where;
   size_t m_amount;
   char m_fill;
   
   FmtAlign(internal::FormatAdapter &adapter, AlignStyle where, size_t amount,
            char fill = ' ')
      : m_adapter(adapter), m_where(where), m_amount(amount), m_fill(fill)
   {}
   
   void format(RawOutStream &stream, StringRef options)
   {
      // If we don't need to align, we can format straight into the underlying
      // stream.  Otherwise we have to go through an intermediate stream first
      // in order to calculate how long the output is so we can align it.
      // TODO: Make the format method return the number of bytes written, that
      // way we can also skip the intermediate stream for left-aligned output.
      if (m_amount == 0) {
         m_adapter.format(stream, options);
         return;
      }
      SmallString<64> item;
      RawSvectorOstream svtream(item);
      
      m_adapter.format(svtream, options);
      if (m_amount <= item.getSize()) {
         stream << item;
         return;
      }
      
      size_t padAmount = m_amount - item.size();
      switch (where) {
      case AlignStyle::Left:
         stream << item;
         fill(stream, padAmount);
         break;
      case AlignStyle::Center: {
         size_t x = padAmount / 2;
         fill(stream, x);
         stream << item;
         fill(stream, padAmount - x);
         break;
      }
      default:
         fill(stream, padAmount);
         stream << item;
         break;
      }
   }
   
private:
   void fill(RawOutStream &outStream, uint32_t count)
   {
      for (uint32_t index = 0; index < count; ++index) {
         outStream << m_fill;
      }
   }
};

} // utils
} // polar

#endif // POLAR_UTILS_FORMAT_COMMON_H
