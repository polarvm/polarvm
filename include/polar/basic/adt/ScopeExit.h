// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/06/26.

#ifndef POLAR_BASIC_ADT_SCOPE_EXIT_H
#define POLAR_BASIC_ADT_SCOPE_EXIT_H

#include "polar/global/Global.h"
#include <type_traits>
#include <utility>

namespace polar {
namespace basic {

namespace internal {

template <typename Callable>
class ScopeExit
{
   Callable m_exitFunction;
   bool m_engaged = true; // False once moved-from or release()d.

public:
   template <typename Fp>
   explicit ScopeExit(Fp &&F) : m_exitFunction(std::forward<Fp>(F))
   {}

   ScopeExit(ScopeExit &&other)
      : m_exitFunction(std::move(Rhs.m_exitFunction)), m_engaged(other.m_engaged)
   {
      other.release();
   }
   ScopeExit(const ScopeExit &) = delete;
   ScopeExit &operator=(ScopeExit &&) = delete;
   ScopeExit &operator=(const ScopeExit &) = delete;

   void release()
   {
      m_engaged = false;
   }

   ~ScopeExit()
   {
      if (m_engaged) {
         m_exitFunction();
      }
   }
};

} // end namespace internal

// Keeps the callable object that is passed in, and execute it at the
// destruction of the returned object (usually at the scope exit where the
// returned object is kept).
//
// Interface is specified by p0052r2.
template <typename Callable>
POLAR_NODISCARD internal::ScopeExit<typename std::decay<Callable>::type>
make_ScopeExit(Callable &&func)
{
   return internal::ScopeExit<typename std::decay<Callable>::type>(
            std::forward<Callable>(func));
}

} // basic
} // polar

#endif // POLAR_BASIC_ADT_SCOPE_EXIT_H
