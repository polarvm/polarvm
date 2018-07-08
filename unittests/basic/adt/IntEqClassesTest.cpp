// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/07/08.

#include "polar/basic/adt/IntEqClasses.h"
#include "gtest/gtest.h"

using namespace polar::basic;

namespace {

TEST(IntEqClassesTest, testSimple)
{
   IntEqClasses ec(10);

   ec.join(0, 1);
   ec.join(3, 2);
   ec.join(4, 5);
   ec.join(7, 6);

   EXPECT_EQ(0u, ec.findLeader(0));
   EXPECT_EQ(0u, ec.findLeader(1));
   EXPECT_EQ(2u, ec.findLeader(2));
   EXPECT_EQ(2u, ec.findLeader(3));
   EXPECT_EQ(4u, ec.findLeader(4));
   EXPECT_EQ(4u, ec.findLeader(5));
   EXPECT_EQ(6u, ec.findLeader(6));
   EXPECT_EQ(6u, ec.findLeader(7));
   EXPECT_EQ(8u, ec.findLeader(8));
   EXPECT_EQ(9u, ec.findLeader(9));

   // join two non-leaders.
   ec.join(1, 3);

   EXPECT_EQ(0u, ec.findLeader(0));
   EXPECT_EQ(0u, ec.findLeader(1));
   EXPECT_EQ(0u, ec.findLeader(2));
   EXPECT_EQ(0u, ec.findLeader(3));
   EXPECT_EQ(4u, ec.findLeader(4));
   EXPECT_EQ(4u, ec.findLeader(5));
   EXPECT_EQ(6u, ec.findLeader(6));
   EXPECT_EQ(6u, ec.findLeader(7));
   EXPECT_EQ(8u, ec.findLeader(8));
   EXPECT_EQ(9u, ec.findLeader(9));

   // join two leaders.
   ec.join(4, 8);

   EXPECT_EQ(0u, ec.findLeader(0));
   EXPECT_EQ(0u, ec.findLeader(1));
   EXPECT_EQ(0u, ec.findLeader(2));
   EXPECT_EQ(0u, ec.findLeader(3));
   EXPECT_EQ(4u, ec.findLeader(4));
   EXPECT_EQ(4u, ec.findLeader(5));
   EXPECT_EQ(6u, ec.findLeader(6));
   EXPECT_EQ(6u, ec.findLeader(7));
   EXPECT_EQ(4u, ec.findLeader(8));
   EXPECT_EQ(9u, ec.findLeader(9));

   // join mixed.
   ec.join(9, 1);

   EXPECT_EQ(0u, ec.findLeader(0));
   EXPECT_EQ(0u, ec.findLeader(1));
   EXPECT_EQ(0u, ec.findLeader(2));
   EXPECT_EQ(0u, ec.findLeader(3));
   EXPECT_EQ(4u, ec.findLeader(4));
   EXPECT_EQ(4u, ec.findLeader(5));
   EXPECT_EQ(6u, ec.findLeader(6));
   EXPECT_EQ(6u, ec.findLeader(7));
   EXPECT_EQ(4u, ec.findLeader(8));
   EXPECT_EQ(0u, ec.findLeader(9));

   // compressed map.
   ec.compress();
   EXPECT_EQ(3u, ec.getNumClasses());

   EXPECT_EQ(0u, ec[0]);
   EXPECT_EQ(0u, ec[1]);
   EXPECT_EQ(0u, ec[2]);
   EXPECT_EQ(0u, ec[3]);
   EXPECT_EQ(1u, ec[4]);
   EXPECT_EQ(1u, ec[5]);
   EXPECT_EQ(2u, ec[6]);
   EXPECT_EQ(2u, ec[7]);
   EXPECT_EQ(1u, ec[8]);
   EXPECT_EQ(0u, ec[9]);

   // uncompressed map.
   ec.uncompress();
   EXPECT_EQ(0u, ec.findLeader(0));
   EXPECT_EQ(0u, ec.findLeader(1));
   EXPECT_EQ(0u, ec.findLeader(2));
   EXPECT_EQ(0u, ec.findLeader(3));
   EXPECT_EQ(4u, ec.findLeader(4));
   EXPECT_EQ(4u, ec.findLeader(5));
   EXPECT_EQ(6u, ec.findLeader(6));
   EXPECT_EQ(6u, ec.findLeader(7));
   EXPECT_EQ(4u, ec.findLeader(8));
   EXPECT_EQ(0u, ec.findLeader(9));
}

} // end anonymous namespace

