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

#ifndef POLAR_UTILS_JAMCRC_H
#define POLAR_UTILS_JAMCRC_H

#include "polar/global/DataTypes.h"

namespace polar {

// forard declare class with namespace
namespace basic {
template <typename T> class ArrayRef;
} // basic

namespace utils {

using polar::basic::ArrayRef;

class JamCRC
{
public:
   JamCRC(uint32_t init = 0xFFFFFFFFU) : m_crc(init)
   {}

   // \brief Update the m_crc calculation with data.
   void update(ArrayRef<char> data);

   uint32_t getCRC() const
   {
      return m_crc;
   }

private:
   uint32_t m_crc;
};

} // utils
} // polar

#endif // POLAR_UTILS_JAMCRC_H
