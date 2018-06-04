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

#ifndef POLAR_BASIC_STRING_SWITCH_H
#define POLAR_BASIC_STRING_SWITCH_H

#include "polar/basic/adt/StringRef.h"
#include <cassert>
#include <cstring>

namespace polar {
namespace basic {

/// \brief A switch()-like statement whose conds are string literals.
///
/// The StringSwitch class is a simple form of a switch() statement that
/// determines whether the given string matches one of the given string
/// literals. The template type parameter \p T is the type of the value that
/// will be returned from the string-switch expression. For example,
/// the following code switches on the name of a color in \c argv[i]:
///
/// \code
/// Color color = StringSwitch<Color>(argv[i])
///   .cond("red", Red)
///   .cond("orange", Orange)
///   .cond("yellow", Yellow)
///   .cond("green", Green)
///   .cond("blue", Blue)
///   .cond("indigo", Indigo)
///   .conds("violet", "purple", Violet)
///   .defaultCond(UnknownColor);
/// \endcode
template<typename T, typename R = T>
class StringSwitch
{
   /// \brief The string we are matching.
   StringRef m_str;
   
   /// \brief The pointer to the result of this switch statement, once known,
   /// null before that.
   const T *m_result;
   
public:
   POLAR_ATTRIBUTE_ALWAYS_INLINE
   explicit StringSwitch(StringRef str)
      : m_str(str), m_result(nullptr)
   {}
   
   // StringSwitch is not copyable.
   StringSwitch(const StringSwitch &) = delete;
   void operator=(const StringSwitch &) = delete;
   
   StringSwitch(StringSwitch &&other)
   {
      *this = std::move(other);
   }
   
   StringSwitch &operator=(StringSwitch &&other)
   {
      m_str = other.m_str;
      m_result = other.m_result;
      return *this;
   }
   
   ~StringSwitch() = default;
   
   // cond-sensitive cond matchers
   template<unsigned N>
   POLAR_ATTRIBUTE_ALWAYS_INLINE
   StringSwitch& cond(const char (&str)[N], const T& value)
   {
      assert(N);
      if (!m_result && N-1 == m_str.getSize() &&
          (N == 1 || std::memcmp(str, m_str.getData(), N-1) == 0))
      {
         m_result = &value;
      }
      return *this;
   }
   
   template<unsigned N>
   POLAR_ATTRIBUTE_ALWAYS_INLINE
   StringSwitch& endsWith(const char (&str)[N], const T &value)
   {
      assert(N);
      if (!m_result && m_str.getSize() >= N-1 &&
          (N == 1 || std::memcmp(str, m_str.getData() + m_str.getSize() + 1 - N, N-1) == 0)) {
         m_result = &value;
      }
      return *this;
   }
   
   template<unsigned N>
   POLAR_ATTRIBUTE_ALWAYS_INLINE
   StringSwitch& startsWith(const char (&str)[N], const T &value)
   {
      assert(N);
      if (!m_result && m_str.getSize() >= N-1 &&
          (N == 1 || std::memcmp(str, m_str.getData(), N-1) == 0)) {
         m_result = &value;
      }
      return *this;
   }
   
   template<unsigned N0, unsigned N1>
   POLAR_ATTRIBUTE_ALWAYS_INLINE
   StringSwitch &conds(const char (&str0)[N0], const char (&str1)[N1],
                       const T& value)
   {
      return cond(str0, value).cond(str1, value);
   }
   
   template<unsigned N0, unsigned N1, unsigned N2>
   POLAR_ATTRIBUTE_ALWAYS_INLINE
   StringSwitch &conds(const char (&str0)[N0], const char (&str1)[N1],
                       const char (&str2)[N2], const T& value)
   {
      return cond(str0, value).conds(str1, str2, value);
   }
   
   template<unsigned N0, unsigned N1, unsigned N2, unsigned N3>
   POLAR_ATTRIBUTE_ALWAYS_INLINE
   StringSwitch &conds(const char (&str0)[N0], const char (&str1)[N1],
                       const char (&str2)[N2], const char (&str3)[N3],
                       const T& value)
   {
      return cond(str0, value).conds(str1, str2, str3, value);
   }
   
   template<unsigned N0, unsigned N1, unsigned N2, unsigned N3, unsigned N4>
   POLAR_ATTRIBUTE_ALWAYS_INLINE
   StringSwitch &conds(const char (&str0)[N0], const char (&str1)[N1],
                       const char (&str2)[N2], const char (&str3)[N3],
                       const char (&str4)[N4], const T& value)
   {
      return cond(str0, value).conds(str1, str2, str3, str4, value);
   }
   
   template <unsigned N0, unsigned N1, unsigned N2, unsigned N3, unsigned N4,
             unsigned N5>
   POLAR_ATTRIBUTE_ALWAYS_INLINE
   StringSwitch &conds(const char (&str0)[N0], const char (&str1)[N1],
                       const char (&str2)[N2], const char (&str3)[N3],
                       const char (&str4)[N4], const char (&str5)[N5],
                       const T &value)
   {
      return cond(str0, value).conds(str1, str2, str3, str4, str5, value);
   }
   
   template <unsigned N0, unsigned N1, unsigned N2, unsigned N3, unsigned N4,
             unsigned N5, unsigned N6>
   POLAR_ATTRIBUTE_ALWAYS_INLINE
   StringSwitch &conds(const char (&str0)[N0], const char (&str1)[N1],
                       const char (&str2)[N2], const char (&str3)[N3],
                       const char (&str4)[N4], const char (&str5)[N5],
                       const char (&str6)[N6], const T &value)
   {
      return cond(str0, value).conds(str1, str2, str3, str4, str5, str6, value);
   }
   
   template <unsigned N0, unsigned N1, unsigned N2, unsigned N3, unsigned N4,
             unsigned N5, unsigned N6, unsigned N7>
   POLAR_ATTRIBUTE_ALWAYS_INLINE
   StringSwitch &conds(const char (&str0)[N0], const char (&str1)[N1],
                       const char (&str2)[N2], const char (&str3)[N3],
                       const char (&str4)[N4], const char (&str5)[N5],
                       const char (&str6)[N6], const char (&str7)[N7],
                       const T &value)
   {
      return cond(str0, value).conds(str1, str2, str3, str4, str5, str6, str7, value);
   }
   
   template <unsigned N0, unsigned N1, unsigned N2, unsigned N3, unsigned N4,
             unsigned N5, unsigned N6, unsigned N7, unsigned N8>
   POLAR_ATTRIBUTE_ALWAYS_INLINE
   StringSwitch &conds(const char (&str0)[N0], const char (&str1)[N1],
                       const char (&str2)[N2], const char (&str3)[N3],
                       const char (&str4)[N4], const char (&str5)[N5],
                       const char (&str6)[N6], const char (&str7)[N7],
                       const char (&str8)[N8], const T &value)
   {
      return cond(str0, value).conds(str1, str2, str3, str4, str5, str6, str7, str8, value);
   }
   
   template <unsigned N0, unsigned N1, unsigned N2, unsigned N3, unsigned N4,
             unsigned N5, unsigned N6, unsigned N7, unsigned N8, unsigned N9>
   POLAR_ATTRIBUTE_ALWAYS_INLINE
   StringSwitch &conds(const char (&str0)[N0], const char (&str1)[N1],
                       const char (&str2)[N2], const char (&str3)[N3],
                       const char (&str4)[N4], const char (&str5)[N5],
                       const char (&str6)[N6], const char (&str7)[N7],
                       const char (&str8)[N8], const char (&S9)[N9],
                       const T &value)
   {
      return cond(str0, value).conds(str1, str2, str3, str4, str5, str6, str7, str8, S9, value);
   }
   
   // cond-insensitive cond matchers.
   template <unsigned N>
   POLAR_ATTRIBUTE_ALWAYS_INLINE StringSwitch &condLower(const char (&str)[N],
                                                         const T &value)
   {
      if (!m_result && m_str.equalsLower(StringRef(str, N - 1))) {
         m_result = &value;
      }
      return *this;
   }
   
   template <unsigned N>
   POLAR_ATTRIBUTE_ALWAYS_INLINE StringSwitch &endsWithLower(const char (&str)[N],
                                                             const T &value)
   {
      if (!m_result && m_str.endsWithLower(StringRef(str, N - 1))) {
         m_result = &value;
      }
      return *this;
   }
   
   template <unsigned N>
   POLAR_ATTRIBUTE_ALWAYS_INLINE StringSwitch &startsWithLower(const char (&str)[N],
                                                               const T &value)
   {
      if (!m_result && m_str.startsWithLower(StringRef(str, N - 1))) {
         m_result = &value;
      }
      return *this;
   }
   
   template <unsigned N0, unsigned N1>
   POLAR_ATTRIBUTE_ALWAYS_INLINE StringSwitch &
   condsLower(const char (&str0)[N0], const char (&str1)[N1], const T &value)
   {
      return condLower(str0, value).condLower(str1, value);
   }
   
   template <unsigned N0, unsigned N1, unsigned N2>
   POLAR_ATTRIBUTE_ALWAYS_INLINE StringSwitch &
   condsLower(const char (&str0)[N0], const char (&str1)[N1], const char (&str2)[N2],
              const T &value)
   {
      return condLower(str0, value).condsLower(str1, str2, value);
   }
   
   template <unsigned N0, unsigned N1, unsigned N2, unsigned N3>
   POLAR_ATTRIBUTE_ALWAYS_INLINE StringSwitch &
   condsLower(const char (&str0)[N0], const char (&str1)[N1], const char (&str2)[N2],
              const char (&str3)[N3], const T &value)
   {
      return condLower(str0, value).condsLower(str1, str2, str3, value);
   }
   
   template <unsigned N0, unsigned N1, unsigned N2, unsigned N3, unsigned N4>
   POLAR_ATTRIBUTE_ALWAYS_INLINE StringSwitch &
   condsLower(const char (&str0)[N0], const char (&str1)[N1], const char (&str2)[N2],
              const char (&str3)[N3], const char (&str4)[N4], const T &value)
   {
      return condLower(str0, value).condsLower(str1, str2, str3, str4, value);
   }
   
   POLAR_ATTRIBUTE_ALWAYS_INLINE
   R defaultCond(const T &value) const
   {
      if (m_result) {
         return *m_result;
      }
      return value;
   }
   
   POLAR_ATTRIBUTE_ALWAYS_INLINE
   operator R() const
   {
      assert(m_result && "Fell off the end of a string-switch");
      return *m_result;
   }
};

} // basic
} // polar

#endif // POLAR_BASIC_STRING_SWITCH_H
