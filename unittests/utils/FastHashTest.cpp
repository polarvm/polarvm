// This source file is part of the polarphp.org open source project

// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception

// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors

// Created by softboy on 2018/07/10.

#include "polar/utils/FastHash.h"
#include "gtest/gtest.h"

using namespace polar::utils;

namespace {

TEST(FastHashTest, testBasic)
{
   EXPECT_EQ(0x33bf00a859c4ba3fU, fast_hash64("foo"));
   EXPECT_EQ(0x48a37c90ad27a659U, fast_hash64("bar"));
   EXPECT_EQ(0x69196c1b3af0bff9U,
             fast_hash64("0123456789abcdefghijklmnopqrstuvwxyz"));
}

} // anonymous namespace
