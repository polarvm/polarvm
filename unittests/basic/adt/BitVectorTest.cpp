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
// Some of these tests fail on PowerPC for unknown reasons.
#ifndef __ppc__

#include "polar/basic/adt/BitVector.h"
#include "polar/basic/adt/SmallBitVector.h"
#include "gtest/gtest.h"

namespace {
template <typename T>
class BitVectorTest : public ::testing::Test
{};

using polar::basic::BitVector;
using polar::basic::SmallBitVector;

// Test both BitVector and SmallBitVector with the same suite of tests.
typedef ::testing::Types<BitVector, SmallBitVector> BitVectorTestTypes;
TYPED_TEST_CASE(BitVectorTest, BitVectorTestTypes);

TYPED_TEST(BitVectorTest, TrivialOperation) {
   TypeParam Vec;
   EXPECT_EQ(0U, Vec.count());
   EXPECT_EQ(0U, Vec.size());
   EXPECT_FALSE(Vec.any());
   EXPECT_TRUE(Vec.all());
   EXPECT_TRUE(Vec.none());
   EXPECT_TRUE(Vec.empty());

   Vec.resize(5, true);
   EXPECT_EQ(5U, Vec.count());
   EXPECT_EQ(5U, Vec.size());
   EXPECT_TRUE(Vec.any());
   EXPECT_TRUE(Vec.all());
   EXPECT_FALSE(Vec.none());
   EXPECT_FALSE(Vec.empty());

   Vec.resize(11);
   EXPECT_EQ(5U, Vec.count());
   EXPECT_EQ(11U, Vec.size());
   EXPECT_TRUE(Vec.any());
   EXPECT_FALSE(Vec.all());
   EXPECT_FALSE(Vec.none());
   EXPECT_FALSE(Vec.empty());

   TypeParam Inv = Vec;
   Inv.flip();
   EXPECT_EQ(6U, Inv.count());
   EXPECT_EQ(11U, Inv.size());
   EXPECT_TRUE(Inv.any());
   EXPECT_FALSE(Inv.all());
   EXPECT_FALSE(Inv.none());
   EXPECT_FALSE(Inv.empty());

   EXPECT_FALSE(Inv == Vec);
   EXPECT_TRUE(Inv != Vec);
   Vec.flip();
   EXPECT_TRUE(Inv == Vec);
   EXPECT_FALSE(Inv != Vec);

   // Add some "interesting" data to Vec.
   Vec.resize(23, true);
   Vec.resize(25, false);
   Vec.resize(26, true);
   Vec.resize(29, false);
   Vec.resize(33, true);
   Vec.resize(57, false);
   unsigned Count = 0;
   for (unsigned i = Vec.findFirst(); i != -1u; i = Vec.findNext(i)) {
      ++Count;
      EXPECT_TRUE(Vec[i]);
      EXPECT_TRUE(Vec.test(i));
   }
   EXPECT_EQ(Count, Vec.count());
   EXPECT_EQ(Count, 23u);
   EXPECT_FALSE(Vec[0]);
   EXPECT_TRUE(Vec[32]);
   EXPECT_FALSE(Vec[56]);
   Vec.resize(61, false);

   TypeParam Copy = Vec;
   TypeParam Alt(3, false);
   Alt.resize(6, true);
   std::swap(Alt, Vec);
   EXPECT_TRUE(Copy == Alt);
   EXPECT_TRUE(Vec.size() == 6);
   EXPECT_TRUE(Vec.count() == 3);
   EXPECT_TRUE(Vec.findFirst() == 3);
   std::swap(Copy, Vec);

   // Add some more "interesting" data.
   Vec.resize(68, true);
   Vec.resize(78, false);
   Vec.resize(89, true);
   Vec.resize(90, false);
   Vec.resize(91, true);
   Vec.resize(130, false);
   Count = 0;
   for (unsigned i = Vec.findFirst(); i != -1u; i = Vec.findNext(i)) {
      ++Count;
      EXPECT_TRUE(Vec[i]);
      EXPECT_TRUE(Vec.test(i));
   }
   EXPECT_EQ(Count, Vec.count());
   EXPECT_EQ(Count, 42u);
   EXPECT_FALSE(Vec[0]);
   EXPECT_TRUE(Vec[32]);
   EXPECT_FALSE(Vec[60]);
   EXPECT_FALSE(Vec[129]);

   Vec.flip(60);
   EXPECT_TRUE(Vec[60]);
   EXPECT_EQ(Count + 1, Vec.count());
   Vec.flip(60);
   EXPECT_FALSE(Vec[60]);
   EXPECT_EQ(Count, Vec.count());

   Vec.reset(32);
   EXPECT_FALSE(Vec[32]);
   EXPECT_EQ(Count - 1, Vec.count());
   Vec.set(32);
   EXPECT_TRUE(Vec[32]);
   EXPECT_EQ(Count, Vec.count());

   Vec.flip();
   EXPECT_EQ(Vec.size() - Count, Vec.count());

   Vec.reset();
   EXPECT_EQ(0U, Vec.count());
   EXPECT_EQ(130U, Vec.size());
   EXPECT_FALSE(Vec.any());
   EXPECT_FALSE(Vec.all());
   EXPECT_TRUE(Vec.none());
   EXPECT_FALSE(Vec.empty());

   Vec.flip();
   EXPECT_EQ(130U, Vec.count());
   EXPECT_EQ(130U, Vec.size());
   EXPECT_TRUE(Vec.any());
   EXPECT_TRUE(Vec.all());
   EXPECT_FALSE(Vec.none());
   EXPECT_FALSE(Vec.empty());

   Vec.resize(64);
   EXPECT_EQ(64U, Vec.count());
   EXPECT_EQ(64U, Vec.size());
   EXPECT_TRUE(Vec.any());
   EXPECT_TRUE(Vec.all());
   EXPECT_FALSE(Vec.none());
   EXPECT_FALSE(Vec.empty());

   Vec.flip();
   EXPECT_EQ(0U, Vec.count());
   EXPECT_EQ(64U, Vec.size());
   EXPECT_FALSE(Vec.any());
   EXPECT_FALSE(Vec.all());
   EXPECT_TRUE(Vec.none());
   EXPECT_FALSE(Vec.empty());

   Inv = TypeParam().flip();
   EXPECT_EQ(0U, Inv.count());
   EXPECT_EQ(0U, Inv.size());
   EXPECT_FALSE(Inv.any());
   EXPECT_TRUE(Inv.all());
   EXPECT_TRUE(Inv.none());
   EXPECT_TRUE(Inv.empty());

   Vec.clear();
   EXPECT_EQ(0U, Vec.count());
   EXPECT_EQ(0U, Vec.size());
   EXPECT_FALSE(Vec.any());
   EXPECT_TRUE(Vec.all());
   EXPECT_TRUE(Vec.none());
   EXPECT_TRUE(Vec.empty());
}

TYPED_TEST(BitVectorTest, SimpleFindOps) {
  // Test finding in an empty BitVector.
  TypeParam A;
  EXPECT_EQ(-1, A.findFirst());
  EXPECT_EQ(-1, A.findLast());
  EXPECT_EQ(-1, A.findFirstUnset());
  EXPECT_EQ(-1, A.findLastUnset());

  // Test finding next set and unset bits in a BitVector with multiple words
  A.resize(100);
  A.set(12);
  A.set(13);
  A.set(75);

  EXPECT_EQ(75, A.findLast());
  EXPECT_EQ(12, A.findFirst());
  EXPECT_EQ(13, A.findNext(12));
  EXPECT_EQ(75, A.findNext(13));
  EXPECT_EQ(-1, A.findNext(75));

  EXPECT_EQ(-1, A.findPrev(12));
  EXPECT_EQ(12, A.findPrev(13));
  EXPECT_EQ(13, A.findPrev(75));
  EXPECT_EQ(75, A.findPrev(90));

  EXPECT_EQ(0, A.findFirstUnset());
  EXPECT_EQ(99, A.findLastUnset());
  EXPECT_EQ(14, A.findNextUnset(11));
  EXPECT_EQ(14, A.findNextUnset(12));
  EXPECT_EQ(14, A.findNextUnset(13));
  EXPECT_EQ(16, A.findNextUnset(15));
  EXPECT_EQ(76, A.findNextUnset(74));
  EXPECT_EQ(76, A.findNextUnset(75));
  EXPECT_EQ(-1, A.findNextUnset(99));

  A.set(0, 100);
  EXPECT_EQ(100U, A.count());
  EXPECT_EQ(0, A.findFirst());
  EXPECT_EQ(-1, A.findFirstUnset());
  EXPECT_EQ(-1, A.findLastUnset());
  EXPECT_EQ(99, A.findLast());
  EXPECT_EQ(99, A.findNext(98));

  A.reset(0, 100);
  EXPECT_EQ(0U, A.count());
  EXPECT_EQ(-1, A.findFirst());
  EXPECT_EQ(-1, A.findLast());
  EXPECT_EQ(0, A.findFirstUnset());
  EXPECT_EQ(99, A.findLastUnset());
  EXPECT_EQ(99, A.findNextUnset(98));

  // Also test with a vector that is small enough to fit in 1 word.
  A.resize(20);
  A.set(3);
  A.set(4);
  A.set(16);
  EXPECT_EQ(16, A.findLast());
  EXPECT_EQ(3, A.findFirst());
  EXPECT_EQ(3, A.findNext(1));
  EXPECT_EQ(4, A.findNext(3));
  EXPECT_EQ(16, A.findNext(4));
  EXPECT_EQ(-1, A.findNext(16));

  EXPECT_EQ(-1, A.findPrev(3));
  EXPECT_EQ(3, A.findPrev(4));
  EXPECT_EQ(4, A.findPrev(16));
  EXPECT_EQ(16, A.findPrev(18));

  EXPECT_EQ(0, A.findFirstUnset());
  EXPECT_EQ(19, A.findLastUnset());
  EXPECT_EQ(5, A.findNextUnset(3));
  EXPECT_EQ(5, A.findNextUnset(4));
  EXPECT_EQ(13, A.findNextUnset(12));
  EXPECT_EQ(17, A.findNextUnset(15));
}

TEST(BitVectorTest, FindInRangeMultiWord) {
  BitVector Vec;

  Vec.resize(200);
  Vec.set(3, 7);
  Vec.set(24, 35);
  Vec.set(50, 70);
  Vec.set(150);
  Vec.set(152);
  Vec.set(154);

  // find first
  EXPECT_EQ(-1, Vec.findFirstIn(0, 0));
  EXPECT_EQ(-1, Vec.findFirstIn(24, 24));
  EXPECT_EQ(-1, Vec.findFirstIn(7, 24));

  EXPECT_EQ(3, Vec.findFirstIn(0, 10));
  EXPECT_EQ(4, Vec.findFirstIn(4, 10));
  EXPECT_EQ(150, Vec.findFirstIn(100, 200));
  EXPECT_EQ(152, Vec.findFirstIn(151, 200));
  EXPECT_EQ(154, Vec.findFirstIn(153, 200));

  EXPECT_EQ(-1, Vec.findFirstIn(155, 200));
  Vec.set(199);
  EXPECT_EQ(199, Vec.findFirstIn(199, 200));
  Vec.reset(199);

  // find last
  EXPECT_EQ(-1, Vec.findLastIn(0, 0));
  EXPECT_EQ(-1, Vec.findLastIn(24, 24));
  EXPECT_EQ(-1, Vec.findLastIn(7, 24));

  EXPECT_EQ(6, Vec.findLastIn(0, 10));
  EXPECT_EQ(5, Vec.findLastIn(0, 6));
  EXPECT_EQ(154, Vec.findLastIn(100, 155));
  EXPECT_EQ(152, Vec.findLastIn(100, 154));
  EXPECT_EQ(150, Vec.findLastIn(100, 152));
  EXPECT_EQ(-1, Vec.findLastIn(100, 150));
  Vec.set(199);
  EXPECT_EQ(199, Vec.findLastIn(199, 200));
  Vec.reset(199);

  // find first unset
  EXPECT_EQ(-1, Vec.findFirstUnsetIn(0, 0));
  EXPECT_EQ(-1, Vec.findFirstUnsetIn(23, 23));
  EXPECT_EQ(-1, Vec.findFirstUnsetIn(24, 35));

  EXPECT_EQ(0, Vec.findFirstUnsetIn(0, 10));
  EXPECT_EQ(1, Vec.findFirstUnsetIn(1, 10));
  EXPECT_EQ(7, Vec.findFirstUnsetIn(5, 25));
  EXPECT_EQ(151, Vec.findFirstUnsetIn(150, 200));
  EXPECT_EQ(151, Vec.findFirstUnsetIn(151, 200));
  EXPECT_EQ(153, Vec.findFirstUnsetIn(152, 200));
  EXPECT_EQ(153, Vec.findFirstUnsetIn(153, 200));
  EXPECT_EQ(155, Vec.findFirstUnsetIn(154, 200));
  EXPECT_EQ(155, Vec.findFirstUnsetIn(155, 200));
  EXPECT_EQ(199, Vec.findFirstUnsetIn(199, 200));

  // find last unset
  EXPECT_EQ(-1, Vec.findLastUnsetIn(0, 0));
  EXPECT_EQ(-1, Vec.findLastUnsetIn(23, 23));
  EXPECT_EQ(-1, Vec.findLastUnsetIn(24, 35));

  EXPECT_EQ(9, Vec.findLastUnsetIn(0, 10));
  EXPECT_EQ(8, Vec.findLastUnsetIn(0, 9));
  EXPECT_EQ(2, Vec.findLastUnsetIn(0, 7));
  EXPECT_EQ(149, Vec.findLastUnsetIn(100, 151));
  EXPECT_EQ(151, Vec.findLastUnsetIn(100, 152));
  EXPECT_EQ(151, Vec.findLastUnsetIn(100, 153));
  EXPECT_EQ(153, Vec.findLastUnsetIn(100, 154));
  EXPECT_EQ(153, Vec.findLastUnsetIn(100, 155));
  EXPECT_EQ(155, Vec.findLastUnsetIn(100, 156));
  EXPECT_EQ(199, Vec.findLastUnsetIn(199, 200));
}

TEST(BitVectorTest, FindInRangeSingleWord) {
  // When the bit vector contains only a single word, this is slightly different
  // than when the bit vector contains multiple words, because masks are applied
  // to the front and back of the same word.  So make sure this works.
  BitVector Vec;

  Vec.resize(25);
  Vec.set(2, 4);
  Vec.set(6, 9);
  Vec.set(12, 15);
  Vec.set(19);
  Vec.set(21);
  Vec.set(23);

  // find first
  EXPECT_EQ(-1, Vec.findFirstIn(0, 0));
  EXPECT_EQ(-1, Vec.findFirstIn(24, 24));
  EXPECT_EQ(-1, Vec.findFirstIn(9, 12));

  EXPECT_EQ(2, Vec.findFirstIn(0, 10));
  EXPECT_EQ(6, Vec.findFirstIn(4, 10));
  EXPECT_EQ(19, Vec.findFirstIn(18, 25));
  EXPECT_EQ(21, Vec.findFirstIn(20, 25));
  EXPECT_EQ(23, Vec.findFirstIn(22, 25));
  EXPECT_EQ(-1, Vec.findFirstIn(24, 25));

  // find last
  EXPECT_EQ(-1, Vec.findLastIn(0, 0));
  EXPECT_EQ(-1, Vec.findLastIn(24, 24));
  EXPECT_EQ(-1, Vec.findLastIn(9, 12));

  EXPECT_EQ(8, Vec.findLastIn(0, 10));
  EXPECT_EQ(3, Vec.findLastIn(0, 6));
  EXPECT_EQ(23, Vec.findLastIn(18, 25));
  EXPECT_EQ(21, Vec.findLastIn(18, 23));
  EXPECT_EQ(19, Vec.findLastIn(18, 21));
  EXPECT_EQ(-1, Vec.findLastIn(18, 19));

  // find first unset
  EXPECT_EQ(-1, Vec.findFirstUnsetIn(0, 0));
  EXPECT_EQ(-1, Vec.findFirstUnsetIn(23, 23));
  EXPECT_EQ(-1, Vec.findFirstUnsetIn(6, 9));

  EXPECT_EQ(0, Vec.findFirstUnsetIn(0, 6));
  EXPECT_EQ(1, Vec.findFirstUnsetIn(1, 6));
  EXPECT_EQ(9, Vec.findFirstUnsetIn(7, 13));
  EXPECT_EQ(18, Vec.findFirstUnsetIn(18, 25));
  EXPECT_EQ(20, Vec.findFirstUnsetIn(19, 25));
  EXPECT_EQ(20, Vec.findFirstUnsetIn(20, 25));
  EXPECT_EQ(22, Vec.findFirstUnsetIn(21, 25));
  EXPECT_EQ(22, Vec.findFirstUnsetIn(22, 25));
  EXPECT_EQ(24, Vec.findFirstUnsetIn(23, 25));
  EXPECT_EQ(24, Vec.findFirstUnsetIn(24, 25));

  // find last unset
  EXPECT_EQ(-1, Vec.findLastUnsetIn(0, 0));
  EXPECT_EQ(-1, Vec.findLastUnsetIn(23, 23));
  EXPECT_EQ(-1, Vec.findLastUnsetIn(6, 9));

  EXPECT_EQ(5, Vec.findLastUnsetIn(0, 6));
  EXPECT_EQ(4, Vec.findLastUnsetIn(0, 5));
  EXPECT_EQ(1, Vec.findLastUnsetIn(0, 4));
  EXPECT_EQ(11, Vec.findLastUnsetIn(7, 13));
  EXPECT_EQ(24, Vec.findLastUnsetIn(18, 25));
  EXPECT_EQ(22, Vec.findLastUnsetIn(18, 24));
  EXPECT_EQ(22, Vec.findLastUnsetIn(18, 23));
  EXPECT_EQ(20, Vec.findLastUnsetIn(18, 22));
  EXPECT_EQ(20, Vec.findLastUnsetIn(18, 21));
  EXPECT_EQ(18, Vec.findLastUnsetIn(18, 20));
  EXPECT_EQ(18, Vec.findLastUnsetIn(18, 19));
}

TYPED_TEST(BitVectorTest, CompoundAssignment) {
  TypeParam A;
  A.resize(10);
  A.set(4);
  A.set(7);

  TypeParam B;
  B.resize(50);
  B.set(5);
  B.set(18);

  A |= B;
  EXPECT_TRUE(A.test(4));
  EXPECT_TRUE(A.test(5));
  EXPECT_TRUE(A.test(7));
  EXPECT_TRUE(A.test(18));
  EXPECT_EQ(4U, A.count());
  EXPECT_EQ(50U, A.size());

  B.resize(10);
  B.set();
  B.reset(2);
  B.reset(7);
  A &= B;
  EXPECT_FALSE(A.test(2));
  EXPECT_FALSE(A.test(7));
  EXPECT_EQ(2U, A.count());
  EXPECT_EQ(50U, A.size());

  B.resize(100);
  B.set();

  A ^= B;
  EXPECT_TRUE(A.test(2));
  EXPECT_TRUE(A.test(7));
  EXPECT_EQ(98U, A.count());
  EXPECT_EQ(100U, A.size());
}

TYPED_TEST(BitVectorTest, ProxyIndex) {
  TypeParam Vec(3);
  EXPECT_TRUE(Vec.none());
  Vec[0] = Vec[1] = Vec[2] = true;
  EXPECT_EQ(Vec.size(), Vec.count());
  Vec[2] = Vec[1] = Vec[0] = false;
  EXPECT_TRUE(Vec.none());
}

TYPED_TEST(BitVectorTest, PortableBitMask) {
  TypeParam A;
  const uint32_t Mask1[] = { 0x80000000, 6, 5 };

  A.resize(10);
  A.setBitsInMask(Mask1, 1);
  EXPECT_EQ(10u, A.size());
  EXPECT_FALSE(A.test(0));

  A.resize(32);
  A.setBitsInMask(Mask1, 1);
  EXPECT_FALSE(A.test(0));
  EXPECT_TRUE(A.test(31));
  EXPECT_EQ(1u, A.count());

  A.resize(33);
  A.setBitsInMask(Mask1, 1);
  EXPECT_EQ(1u, A.count());
  A.setBitsInMask(Mask1, 2);
  EXPECT_EQ(1u, A.count());

  A.resize(34);
  A.setBitsInMask(Mask1, 2);
  EXPECT_EQ(2u, A.count());

  A.resize(65);
  A.setBitsInMask(Mask1, 3);
  EXPECT_EQ(4u, A.count());

  A.setBitsNotInMask(Mask1, 1);
  EXPECT_EQ(32u+3u, A.count());

  A.setBitsNotInMask(Mask1, 3);
  EXPECT_EQ(65u, A.count());

  A.resize(96);
  EXPECT_EQ(65u, A.count());

  A.clear();
  A.resize(128);
  A.setBitsNotInMask(Mask1, 3);
  EXPECT_EQ(96u-5u, A.count());

  A.clearBitsNotInMask(Mask1, 1);
  EXPECT_EQ(64-4u, A.count());
}

TYPED_TEST(BitVectorTest, BinOps) {
  TypeParam A;
  TypeParam B;

  A.resize(65);
  EXPECT_FALSE(A.anyCommon(B));
  EXPECT_FALSE(B.anyCommon(B));

  B.resize(64);
  A.set(64);
  EXPECT_FALSE(A.anyCommon(B));
  EXPECT_FALSE(B.anyCommon(A));

  B.set(63);
  EXPECT_FALSE(A.anyCommon(B));
  EXPECT_FALSE(B.anyCommon(A));

  A.set(63);
  EXPECT_TRUE(A.anyCommon(B));
  EXPECT_TRUE(B.anyCommon(A));

  B.resize(70);
  B.set(64);
  B.reset(63);
  A.resize(64);
  EXPECT_FALSE(A.anyCommon(B));
  EXPECT_FALSE(B.anyCommon(A));
}

typedef std::vector<std::pair<int, int>> RangeList;

template <typename VecType>
static inline VecType createBitVector(uint32_t Size,
                                      const RangeList &setRanges) {
  VecType V;
  V.resize(Size);
  for (auto &R : setRanges)
    V.set(R.first, R.second);
  return V;
}

TYPED_TEST(BitVectorTest, ShiftOpsSingleWord) {
  // Test that shift ops work when the desired shift amount is less
  // than one word.

  // 1. Case where the number of bits in the BitVector also fit into a single
  // word.
  TypeParam A = createBitVector<TypeParam>(12, {{2, 4}, {8, 10}});
  TypeParam B = A;

  EXPECT_EQ(4U, A.count());
  EXPECT_TRUE(A.test(2));
  EXPECT_TRUE(A.test(3));
  EXPECT_TRUE(A.test(8));
  EXPECT_TRUE(A.test(9));

  A >>= 1;
  EXPECT_EQ(createBitVector<TypeParam>(12, {{1, 3}, {7, 9}}), A);

  A <<= 1;
  EXPECT_EQ(B, A);

  A >>= 10;
  EXPECT_EQ(createBitVector<TypeParam>(12, {}), A);

  A = B;
  A <<= 10;
  EXPECT_EQ(createBitVector<TypeParam>(12, {}), A);

  // 2. Case where the number of bits in the BitVector do not fit into a single
  // word.

  // 31----------------------------------------------------------------------0
  // XXXXXXXX XXXXXXXX XXXXXXXX 00000111 | 11111110 00000000 00001111 11111111
  A = createBitVector<TypeParam>(40, {{0, 12}, {25, 35}});
  EXPECT_EQ(40U, A.size());
  EXPECT_EQ(22U, A.count());

  // 2a. Make sure that left shifting some 1 bits out of the vector works.
  //   31----------------------------------------------------------------------0
  // Before:
  //   XXXXXXXX XXXXXXXX XXXXXXXX 00000111 | 11111110 00000000 00001111 11111111
  // After:
  //   XXXXXXXX XXXXXXXX XXXXXXXX 11111100 | 00000000 00011111 11111110 00000000
  A <<= 9;
  EXPECT_EQ(createBitVector<TypeParam>(40, {{9, 21}, {34, 40}}), A);

  // 2b. Make sure that keeping the number of one bits unchanged works.
  //   31----------------------------------------------------------------------0
  // Before:
  //   XXXXXXXX XXXXXXXX XXXXXXXX 11111100 | 00000000 00011111 11111110 00000000
  // After:
  //   XXXXXXXX XXXXXXXX XXXXXXXX 00000011 | 11110000 00000000 01111111 11111000
  A >>= 6;
  EXPECT_EQ(createBitVector<TypeParam>(40, {{3, 15}, {28, 34}}), A);

  // 2c. Make sure that right shifting some 1 bits out of the vector works.
  //   31----------------------------------------------------------------------0
  // Before:
  //   XXXXXXXX XXXXXXXX XXXXXXXX 00000011 | 11110000 00000000 01111111 11111000
  // After:
  //   XXXXXXXX XXXXXXXX XXXXXXXX 00000000 | 00000000 11111100 00000000 00011111
  A >>= 10;
  EXPECT_EQ(createBitVector<TypeParam>(40, {{0, 5}, {18, 24}}), A);

  // 3. Big test.
  A = createBitVector<TypeParam>(300, {{1, 30}, {60, 95}, {200, 275}});
  A <<= 29;
  EXPECT_EQ(createBitVector<TypeParam>(
                300, {{1 + 29, 30 + 29}, {60 + 29, 95 + 29}, {200 + 29, 300}}),
            A);
}

TYPED_TEST(BitVectorTest, ShiftOpsMultiWord) {
  // Test that shift ops work when the desired shift amount is greater than or
  // equal to the size of a single word.
  auto A = createBitVector<TypeParam>(300, {{1, 30}, {60, 95}, {200, 275}});

  // Make a copy so we can re-use it later.
  auto B = A;

  // 1. Shift left by an exact multiple of the word size.  This should invoke
  // only a memmove and no per-word bit operations.
  A <<= 64;
  auto Expected = createBitVector<TypeParam>(
      300, {{1 + 64, 30 + 64}, {60 + 64, 95 + 64}, {200 + 64, 300}});
  EXPECT_EQ(Expected, A);

  // 2. Shift left by a non multiple of the word size.  This should invoke both
  // a memmove and per-word bit operations.
  A = B;
  A <<= 93;
  EXPECT_EQ(createBitVector<TypeParam>(
                300, {{1 + 93, 30 + 93}, {60 + 93, 95 + 93}, {200 + 93, 300}}),
            A);

  // 1. Shift right by an exact multiple of the word size.  This should invoke
  // only a memmove and no per-word bit operations.
  A = B;
  A >>= 64;
  EXPECT_EQ(
      createBitVector<TypeParam>(300, {{0, 95 - 64}, {200 - 64, 275 - 64}}), A);

  // 2. Shift left by a non multiple of the word size.  This should invoke both
  // a memmove and per-word bit operations.
  A = B;
  A >>= 93;
  EXPECT_EQ(
      createBitVector<TypeParam>(300, {{0, 95 - 93}, {200 - 93, 275 - 93}}), A);
}

TYPED_TEST(BitVectorTest, RangeOps) {
  TypeParam A;
  A.resize(256);
  A.reset();
  A.set(1, 255);

  EXPECT_FALSE(A.test(0));
  EXPECT_TRUE( A.test(1));
  EXPECT_TRUE( A.test(23));
  EXPECT_TRUE( A.test(254));
  EXPECT_FALSE(A.test(255));

  TypeParam B;
  B.resize(256);
  B.set();
  B.reset(1, 255);

  EXPECT_TRUE( B.test(0));
  EXPECT_FALSE(B.test(1));
  EXPECT_FALSE(B.test(23));
  EXPECT_FALSE(B.test(254));
  EXPECT_TRUE( B.test(255));

  TypeParam C;
  C.resize(3);
  C.reset();
  C.set(0, 1);

  EXPECT_TRUE(C.test(0));
  EXPECT_FALSE( C.test(1));
  EXPECT_FALSE( C.test(2));

  TypeParam D;
  D.resize(3);
  D.set();
  D.reset(0, 1);

  EXPECT_FALSE(D.test(0));
  EXPECT_TRUE( D.test(1));
  EXPECT_TRUE( D.test(2));

  TypeParam E;
  E.resize(128);
  E.reset();
  E.set(1, 33);

  EXPECT_FALSE(E.test(0));
  EXPECT_TRUE( E.test(1));
  EXPECT_TRUE( E.test(32));
  EXPECT_FALSE(E.test(33));

  TypeParam BufferOverrun;
  unsigned size = sizeof(unsigned long) * 8;
  BufferOverrun.resize(size);
  BufferOverrun.reset(0, size);
  BufferOverrun.set(0, size);
}

TYPED_TEST(BitVectorTest, CompoundTestReset) {
  TypeParam A(50, true);
  TypeParam B(50, false);

  TypeParam C(100, true);
  TypeParam D(100, false);

  EXPECT_FALSE(A.test(A));
  EXPECT_TRUE(A.test(B));
  EXPECT_FALSE(A.test(C));
  EXPECT_TRUE(A.test(D));
  EXPECT_FALSE(B.test(A));
  EXPECT_FALSE(B.test(B));
  EXPECT_FALSE(B.test(C));
  EXPECT_FALSE(B.test(D));
  EXPECT_TRUE(C.test(A));
  EXPECT_TRUE(C.test(B));
  EXPECT_FALSE(C.test(C));
  EXPECT_TRUE(C.test(D));

  A.reset(B);
  A.reset(D);
  EXPECT_TRUE(A.all());
  A.reset(A);
  EXPECT_TRUE(A.none());
  A.set();
  A.reset(C);
  EXPECT_TRUE(A.none());
  A.set();

  C.reset(A);
  EXPECT_EQ(50, C.findFirst());
  C.reset(C);
  EXPECT_TRUE(C.none());
}

TYPED_TEST(BitVectorTest, MoveConstructor) {
  TypeParam A(10, true);
  TypeParam B(std::move(A));
  // Check that the move ctor leaves the moved-from object in a valid state.
  // The following line used to crash.
  A = B;

  TypeParam C(10, true);
  EXPECT_EQ(C, A);
  EXPECT_EQ(C, B);
}

TYPED_TEST(BitVectorTest, MoveAssignment) {
  TypeParam A(10, true);
  TypeParam B;
  B = std::move(A);
  // Check that move assignment leaves the moved-from object in a valid state.
  // The following line used to crash.
  A = B;

  TypeParam C(10, true);
  EXPECT_EQ(C, A);
  EXPECT_EQ(C, B);
}

template<class TypeParam>
static void testEmpty(const TypeParam &A) {
  EXPECT_TRUE(A.empty());
  EXPECT_EQ((size_t)0, A.size());
  EXPECT_EQ((size_t)0, A.count());
  EXPECT_FALSE(A.any());
  EXPECT_TRUE(A.all());
  EXPECT_TRUE(A.none());
  EXPECT_EQ(-1, A.findFirst());
  EXPECT_EQ(A, TypeParam());
}

/// Tests whether BitVector behaves well with Bits==nullptr, Capacity==0
TYPED_TEST(BitVectorTest, EmptyVector) {
  TypeParam A;
  testEmpty(A);

  TypeParam B;
  B.reset();
  testEmpty(B);

  TypeParam C;
  C.clear();
  testEmpty(C);

  TypeParam D(A);
  testEmpty(D);

  TypeParam E;
  E = A;
  testEmpty(E);

  TypeParam F;
  E.reset(A);
  testEmpty(E);
}

TYPED_TEST(BitVectorTest, Iterators) {
  TypeParam Filled(10, true);
  EXPECT_NE(Filled.setBitsBegin(), Filled.setBitsEnd());
  unsigned Counter = 0;
  for (unsigned Bit : Filled.setBits())
    EXPECT_EQ(Bit, Counter++);

  TypeParam Empty;
  EXPECT_EQ(Empty.setBitsBegin(), Empty.setBitsEnd());
  for (unsigned Bit : Empty.setBits()) {
    (void)Bit;
    EXPECT_TRUE(false);
  }

  TypeParam ToFill(100, false);
  ToFill.set(0);
  EXPECT_NE(ToFill.setBitsBegin(), ToFill.setBitsEnd());
  EXPECT_EQ(++ToFill.setBitsBegin(), ToFill.setBitsEnd());
  EXPECT_EQ(*ToFill.setBitsBegin(), 0U);
  ToFill.reset(0);
  EXPECT_EQ(ToFill.setBitsBegin(), ToFill.setBitsEnd());

  const unsigned List[] = {1, 10, 25, 99};
  for (unsigned Num : List)
    ToFill.set(Num);
  unsigned i = 0;
  for (unsigned Bit : ToFill.setBits())
    EXPECT_EQ(List[i++], Bit);
}

}

#endif
