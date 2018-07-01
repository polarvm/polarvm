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

#ifndef POLAR_UTILS_SAVE_AND_RESTORE_H
#define POLAR_UTILS_SAVE_AND_RESTORE_H

namespace polar {
namespace utils {

/// A utility class that uses RAII to save and restore the value of a variable.
template <typename T>
struct SaveAndRestore
{
   SaveAndRestore(T &value) : m_value(value), m_oldValue(value)
   {}
   SaveAndRestore(T &value, const T &newValue) : m_value(value), m_oldValue(value)
   {
      m_value = newValue;
   }
   ~SaveAndRestore()
   {
      m_value = m_oldValue;
   }

   T get()
   {
      return m_oldValue;
   }

private:
   T &m_value;
   T m_oldValue;
};

/// Similar to \c SaveAndRestore.  Operates only on bools; the old value of a
/// variable is saved, and during the dstor the old value is or'ed with the new
/// value.
struct SaveOr
{
   SaveOr(bool &value) : m_value(value), m_oldValue(value)
   {
      m_value = false;
   }

   ~SaveOr()
   {
      m_value |= m_oldValue;
   }

private:
   bool &m_value;
   const bool m_oldValue;
};

} // utils
} // polar

#endif // POLAR_UTILS_SAVE_AND_RESTORE_H
