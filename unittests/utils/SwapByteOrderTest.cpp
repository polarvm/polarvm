// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
// 
// Created by softboy on 2018/06/01.

#include "gtest/gtest.h"
#include "polar/utils/SwapByteOrder.h"
#include <cstdlib>
#include <ctime>

using polar::utils::get_swapped_bytes;

TEST(SwapByteOrderTest, testUnsignedRoundTrip)
{
   // The point of the bit twiddling of magic is to test with and without bits
   // in every byte.
   uint64_t value = 1;
   for (std::size_t i = 0; i <= sizeof(value); ++i) {
      uint8_t original_uint8 = static_cast<uint8_t>(value);
      EXPECT_EQ(original_uint8,
                get_swapped_bytes(get_swapped_bytes(original_uint8)));
      
      uint16_t original_uint16 = static_cast<uint16_t>(value);
      EXPECT_EQ(original_uint16,
                get_swapped_bytes(get_swapped_bytes(original_uint16)));
      
      uint32_t original_uint32 = static_cast<uint32_t>(value);
      EXPECT_EQ(original_uint32,
                get_swapped_bytes(get_swapped_bytes(original_uint32)));
      
      uint64_t original_uint64 = static_cast<uint64_t>(value);
      EXPECT_EQ(original_uint64,
                get_swapped_bytes(get_swapped_bytes(original_uint64)));
      
      value = (value << 8) | 0x55; // binary 0101 0101.
   }
}

