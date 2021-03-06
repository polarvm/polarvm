// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/07/15.

#include "polar/utils/StringPool.h"
#include "gtest/gtest.h"

using namespace polar;
using namespace polar::basic;
using namespace polar::utils;

namespace {

TEST(PooledStringPtrTest, testOperatorEquals)
{
   StringPool pool;
   const PooledStringPtr a = pool.intern("a");
   const PooledStringPtr b = pool.intern("b");
   EXPECT_FALSE(a == b);
}

TEST(PooledStringPtrTest, testOperatorNotEquals)
{
   StringPool pool;
   const PooledStringPtr a = pool.intern("a");
   const PooledStringPtr b = pool.intern("b");
   EXPECT_TRUE(a != b);
}

} // anonymous namespace
