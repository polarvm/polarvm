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

#include "polar/basic/adt/FoldingSet.h"
#include "polar/basic/adt/StringRef.h"
#include "gtest/gtest.h"
#include <string>
#include <iostream>

namespace {

using polar::basic::FoldingSetNodeId;
using polar::basic::FoldingSetNode;
using polar::basic::FoldingSet;
using polar::basic::StringRef;

// Unaligned string test.
TEST(FoldingSetTest, UnalignedStringTest)
{
   SCOPED_TRACE("UnalignedStringTest");

   FoldingSetNodeId a, b;
   // An aligned string.
   std::string str1= "a test string";
   a.addString(str1);

   // An unaligned string.
   std::string str2 = ">" + str1;
   b.addString((str2.c_str() + 1));
   EXPECT_EQ(a.computeHash(), b.computeHash());
}

TEST(FoldingSetTest, LongLongComparison)
{
   struct LongLongContainer : FoldingSetNode {
      unsigned long long A, B;
      LongLongContainer(unsigned long long A, unsigned long long B)
         : A(A), B(B) {}
      void profile(FoldingSetNodeId &ID) const {
         ID.addInteger(A);
         ID.addInteger(B);
      }
   };

   LongLongContainer C1((1ULL << 32) + 1, 1ULL);
   LongLongContainer C2(1ULL, (1ULL << 32) + 1);

   FoldingSet<LongLongContainer> Set;

   EXPECT_EQ(&C1, Set.getOrInsertNode(&C1));
   EXPECT_EQ(&C2, Set.getOrInsertNode(&C2));
   EXPECT_EQ(2U, Set.size());
}

struct TrivialPair : public FoldingSetNode
{
   unsigned Key = 0;
   unsigned Value = 0;
   TrivialPair(unsigned K, unsigned V) : FoldingSetNode(), Key(K), Value(V) {}

   void profile(FoldingSetNodeId &ID) const {
      ID.addInteger(Key);
      ID.addInteger(Value);
   }
};

TEST(FoldingSetTest, IDComparison)
{
   FoldingSet<TrivialPair> Trivial;

   TrivialPair T(99, 42);
   Trivial.insertNode(&T);

   void *InsertPos = nullptr;
   FoldingSetNodeId ID;
   T.profile(ID);
   TrivialPair *N = Trivial.findNodeOrInsertPos(ID, InsertPos);
   EXPECT_EQ(&T, N);
   EXPECT_EQ(nullptr, InsertPos);
}

TEST(FoldingSetTest, MissedIDComparison)
{
   FoldingSet<TrivialPair> Trivial;

   TrivialPair S(100, 42);
   TrivialPair T(99, 42);
   Trivial.insertNode(&T);

   void *InsertPos = nullptr;
   FoldingSetNodeId ID;
   S.profile(ID);
   TrivialPair *N = Trivial.findNodeOrInsertPos(ID, InsertPos);
   EXPECT_EQ(nullptr, N);
   EXPECT_NE(nullptr, InsertPos);
}

TEST(FoldingSetTest, RemoveNodeThatIsPresent)
{
   FoldingSet<TrivialPair> Trivial;

   TrivialPair T(99, 42);
   Trivial.insertNode(&T);
   EXPECT_EQ(Trivial.size(), 1U);

   bool WasThere = Trivial.removeNode(&T);
   EXPECT_TRUE(WasThere);
   EXPECT_EQ(0U, Trivial.size());
}

TEST(FoldingSetTest, RemoveNodeThatIsAbsent)
{
   FoldingSet<TrivialPair> Trivial;

   TrivialPair T(99, 42);
   bool WasThere = Trivial.removeNode(&T);
   EXPECT_FALSE(WasThere);
   EXPECT_EQ(0U, Trivial.size());
}

TEST(FoldingSetTest, GetOrInsertInserting)
{
   FoldingSet<TrivialPair> Trivial;

   TrivialPair T(99, 42);
   TrivialPair *N = Trivial.getOrInsertNode(&T);
   EXPECT_EQ(&T, N);
}

TEST(FoldingSetTest, GetOrInsertGetting)
{
   FoldingSet<TrivialPair> Trivial;

   TrivialPair T(99, 42);
   TrivialPair T2(99, 42);
   Trivial.insertNode(&T);
   TrivialPair *N = Trivial.getOrInsertNode(&T2);
   EXPECT_EQ(&T, N);
}

TEST(FoldingSetTest, InsertAtPos)
{
   FoldingSet<TrivialPair> Trivial;

   void *InsertPos = nullptr;
   TrivialPair Finder(99, 42);
   FoldingSetNodeId ID;
   Finder.profile(ID);
   Trivial.findNodeOrInsertPos(ID, InsertPos);

   TrivialPair T(99, 42);
   Trivial.insertNode(&T, InsertPos);
   EXPECT_EQ(1U, Trivial.size());
}

TEST(FoldingSetTest, EmptyIsTrue)
{
   FoldingSet<TrivialPair> Trivial;
   EXPECT_TRUE(Trivial.empty());
}

TEST(FoldingSetTest, EmptyIsFalse)
{
   FoldingSet<TrivialPair> Trivial;
   TrivialPair T(99, 42);
   Trivial.insertNode(&T);
   EXPECT_FALSE(Trivial.empty());
}

TEST(FoldingSetTest, ClearOnEmpty)
{
   FoldingSet<TrivialPair> Trivial;
   Trivial.clear();
   EXPECT_TRUE(Trivial.empty());
}

TEST(FoldingSetTest, ClearOnNonEmpty)
{
   FoldingSet<TrivialPair> Trivial;
   TrivialPair T(99, 42);
   Trivial.insertNode(&T);
   Trivial.clear();
   EXPECT_TRUE(Trivial.empty());
}

TEST(FoldingSetTest, CapacityLargerThanReserve)
{
   FoldingSet<TrivialPair> Trivial;
   auto OldCapacity = Trivial.getCapacity();
   Trivial.reserve(OldCapacity + 1);
   EXPECT_GE(Trivial.getCapacity(), OldCapacity + 1);
}

TEST(FoldingSetTest, SmallReserveChangesNothing)
{
   FoldingSet<TrivialPair> Trivial;
   auto OldCapacity = Trivial.getCapacity();
   Trivial.reserve(OldCapacity - 1);
   EXPECT_EQ(Trivial.getCapacity(), OldCapacity);
}

}

