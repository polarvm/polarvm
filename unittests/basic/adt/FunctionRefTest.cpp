// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/06/06.

#include "polar/basic/adt/StlExtras.h"
#include "gtest/gtest.h"

using namespace polar::basic;

namespace {

// Ensure that there is a default constructor and we can test for a null
// FunctionRef.
TEST(FunctionRefTest, testNull)
{
  FunctionRef<int()> F;
  EXPECT_FALSE(F);

  auto L = [] { return 1; };
  F = L;
  EXPECT_TRUE(F);

  F = {};
  EXPECT_FALSE(F);
}

// Ensure that copies of a FunctionRef copy the underlying state rather than
// causing one FunctionRef to chain to the next.
TEST(FunctionRefTest, testCopy) {
  auto A = [] { return 1; };
  auto B = [] { return 2; };
  FunctionRef<int()> X = A;
  FunctionRef<int()> Y = X;
  X = B;
  EXPECT_EQ(1, Y());
}

}
