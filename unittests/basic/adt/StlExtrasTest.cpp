// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/06/02.

#include "gtest/gtest.h"
#include "polar/basic/adt/StlExtras.h"

#include <list>
#include <vector>

namespace {

using polar::basic::rank;

int f(rank<0>) { return 0; }
int f(rank<1>) { return 1; }
int f(rank<2>) { return 2; }
int f(rank<4>) { return 4; }

}

TEST(StlExtrasTest, testRank)
{
   // We shouldn't get ambiguities and should select the overload of the same
   // rank as the argument.
   EXPECT_EQ(0, f(rank<0>()));
   EXPECT_EQ(1, f(rank<1>()));
   EXPECT_EQ(2, f(rank<2>()));
}


