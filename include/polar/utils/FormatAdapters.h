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

#ifndef POLAR_UTILS_FORMAT_ADAPTERS_H
#define POLAR_UTILS_FORMAT_ADAPTERS_H

#include "polar/basic/adt/SmallString.h"
#include "polar/basic/adt/StringRef.h"
#include "polar/utils/FormatCommon.h"
#include "polar/utils/FormatVariadicDetails.h"
#include "polar/utils/RawOutStream.h"

namespace polar {
namespace utils {

template <typename T>
class FormatAdapter : public internal::FormatAdapterImpl
{
protected:
   explicit FormatAdapter(T &&item) : m_item(item)
   {}
   
   T m_item;
};

namespace internal {
template <typename T>
class AlignAdapter final : public FormatAdapter<T>
{
   AlignStyle m_where;
   size_t m_amount;
   char m_fill;
   
public:
   AlignAdapter(T &&item, AlignStyle where, size_t amount, char fill)
      : FormatAdapter<T>(std::forward<T>(item)), m_where(where), m_amount(amount),
        m_fill(fill)
   {}
   
   void format(polar::utils::RawOutStream &stream, StringRef style)
   {
      auto adapter = internal::build_format_adapter(std::forward<T>(this->m_item));
      FmtAlign(adapter, m_where, m_amount, m_fill).format(stream, style);
   }
};

template <typename T>
class PadAdapter final : public FormatAdapter<T>
{
   size_t m_left;
   size_t m_right;
   
public:
   PadAdapter(T &&item, size_t left, size_t right)
      : FormatAdapter<T>(std::forward<T>(item)), m_left(left), m_right(right)
   {}
   
   void format(polar::utils::RawOutStream &stream, StringRef style)
   {
      auto adapter = internal::build_format_adapter(std::forward<T>(this->m_item));
      stream.indent(m_left);
      adapter.format(stream, style);
      stream.indent(m_right);
   }
};

template <typename T>
class RepeatAdapter final : public FormatAdapter<T>
{
   size_t m_count;
   
public:
   RepeatAdapter(T &&item, size_t count)
      : FormatAdapter<T>(std::forward<T>(item)), m_count(count)
   {}
   
   void format(polar::utils::RawOutStream &stream, StringRef style)
   {
      auto adapter = internal::build_format_adapter(std::forward<T>(this->m_item));
      for (size_t index = 0; index < m_count; ++index) {
         adapter.format(stream, style);
      }
   }
};
} // internal

template <typename T>
internal::AlignAdapter<T> fmt_align(T &&item, AlignStyle where, size_t amount,
                                    char fill = ' ')
{
   return internal::AlignAdapter<T>(std::forward<T>(item), where, amount, fill);
}

template <typename T>
internal::PadAdapter<T> fmt_pad(T &&item, size_t left, size_t right)
{
   return internal::PadAdapter<T>(std::forward<T>(item), left, right);
}

template <typename T>
internal::RepeatAdapter<T> fmt_repeat(T &&item, size_t count)
{
   return internal::RepeatAdapter<T>(std::forward<T>(item), count);
}

} // utils
} // polar

#endif // POLAR_UTILS_FORMAT_ADAPTERS_H
