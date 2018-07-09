// This source file is part of the polarphp.org open source project

// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception

// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors

// Created by softboy on 2018/07/09.

#include "polar/basic/adt/SparseBitVector.h"
#include "gtest/gtest.h"

using namespace polar::basic;

namespace {

TEST(SparseBitVectorTest, testTrivialOperation)
{
   SparseBitVector<> vector;
   EXPECT_EQ(0U, vector.count());
   EXPECT_FALSE(vector.test(17));
   vector.set(5);
   EXPECT_TRUE(vector.test(5));
   EXPECT_FALSE(vector.test(17));
   vector.reset(6);
   EXPECT_TRUE(vector.test(5));
   EXPECT_FALSE(vector.test(6));
   vector.reset(5);
   EXPECT_FALSE(vector.test(5));
   EXPECT_TRUE(vector.testAndSet(17));
   EXPECT_FALSE(vector.testAndSet(17));
   EXPECT_TRUE(vector.test(17));
   vector.clear();
   EXPECT_FALSE(vector.test(17));
}

TEST(SparseBitVectorTest, testIntersectWith)
{
   SparseBitVector<> vector, Other;

   vector.set(1);
   Other.set(1);
   EXPECT_FALSE(vector &= Other);
   EXPECT_TRUE(vector.test(1));

   vector.clear();
   vector.set(5);
   Other.clear();
   Other.set(6);
   EXPECT_TRUE(vector &= Other);
   EXPECT_TRUE(vector.empty());

   vector.clear();
   vector.set(5);
   Other.clear();
   Other.set(225);
   EXPECT_TRUE(vector &= Other);
   EXPECT_TRUE(vector.empty());

   vector.clear();
   vector.set(225);
   Other.clear();
   Other.set(5);
   EXPECT_TRUE(vector &= Other);
   EXPECT_TRUE(vector.empty());
}

TEST(SparseBitVectorTest, testSelfAssignment)
{
   SparseBitVector<> vector, Other;

   vector.set(23);
   vector.set(234);
   vector = vector;
   EXPECT_TRUE(vector.test(23));
   EXPECT_TRUE(vector.test(234));

   vector.clear();
   vector.set(17);
   vector.set(256);
   EXPECT_FALSE(vector |= vector);
   EXPECT_TRUE(vector.test(17));
   EXPECT_TRUE(vector.test(256));

   vector.clear();
   vector.set(56);
   vector.set(517);
   EXPECT_FALSE(vector &= vector);
   EXPECT_TRUE(vector.test(56));
   EXPECT_TRUE(vector.test(517));

   vector.clear();
   vector.set(99);
   vector.set(333);
   EXPECT_TRUE(vector.intersectWithComplement(vector));
   EXPECT_TRUE(vector.empty());
   EXPECT_FALSE(vector.intersectWithComplement(vector));

   vector.clear();
   vector.set(28);
   vector.set(43);
   vector.intersectWithComplement(vector, vector);
   EXPECT_TRUE(vector.empty());

   vector.clear();
   vector.set(42);
   vector.set(567);
   Other.set(55);
   Other.set(567);
   vector.intersectWithComplement(vector, Other);
   EXPECT_TRUE(vector.test(42));
   EXPECT_FALSE(vector.test(567));

   vector.clear();
   vector.set(19);
   vector.set(21);
   Other.clear();
   Other.set(19);
   Other.set(31);
   vector.intersectWithComplement(Other, vector);
   EXPECT_FALSE(vector.test(19));
   EXPECT_TRUE(vector.test(31));

   vector.clear();
   vector.set(1);
   Other.clear();
   Other.set(59);
   Other.set(75);
   vector.intersectWithComplement(Other, Other);
   EXPECT_TRUE(vector.empty());
}

TEST(SparseBitVectorTest, testFind)
{
   SparseBitVector<> vector;
   vector.set(1);
   EXPECT_EQ(1, vector.findFirst());
   EXPECT_EQ(1, vector.findLast());

   vector.set(2);
   EXPECT_EQ(1, vector.findFirst());
   EXPECT_EQ(2, vector.findLast());

   vector.set(0);
   vector.set(3);
   EXPECT_EQ(0, vector.findFirst());
   EXPECT_EQ(3, vector.findLast());

   vector.reset(1);
   vector.reset(0);
   vector.reset(3);
   EXPECT_EQ(2, vector.findFirst());
   EXPECT_EQ(2, vector.findLast());

   // Set some large bits to ensure we are pulling bits from more than just a
   // single bitword.
   vector.set(500);
   vector.set(2000);
   vector.set(3000);
   vector.set(4000);
   vector.reset(2);
   EXPECT_EQ(500, vector.findFirst());
   EXPECT_EQ(4000, vector.findLast());

   vector.reset(500);
   vector.reset(3000);
   vector.reset(4000);
   EXPECT_EQ(2000, vector.findFirst());
   EXPECT_EQ(2000, vector.findLast());

   vector.clear();
}

}
