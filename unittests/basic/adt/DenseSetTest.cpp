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

#include "polar/basic/adt/DenseSet.h"
#include "gtest/gtest.h"
#include <type_traits>


namespace {

using namespace polar::basic;

// Test hashing with a set of only two entries.
TEST(DenseSetTest, DoubleEntrySetTest)
{
  DenseSet<unsigned> set(2);
  set.insert(0);
  set.insert(1);
  // Original failure was an infinite loop in this call:
  EXPECT_EQ(0u, set.count(2));
}

struct TestDenseSetInfo
{
  static inline unsigned getEmptyKey() { return ~0; }
  static inline unsigned getTombstoneKey() { return ~0U - 1; }
  static unsigned getHashValue(const unsigned& Val) { return Val * 37U; }
  static unsigned getHashValue(const char* Val) {
    return (unsigned)(Val[0] - 'a') * 37U;
  }
  static bool isEqual(const unsigned& LHS, const unsigned& RHS) {
    return LHS == RHS;
  }
  static bool isEqual(const char* LHS, const unsigned& RHS) {
    return (unsigned)(LHS[0] - 'a') == RHS;
  }
};

// Test fixture
template <typename T> class DenseSetTest : public testing::Test
{
protected:
  T Set = GetTestSet();

private:
  static T GetTestSet() {
    typename std::remove_const<T>::type Set;
    Set.insert(0);
    Set.insert(1);
    Set.insert(2);
    return Set;
  }
};

// Register these types for testing.
typedef ::testing::Types<DenseSet<unsigned, TestDenseSetInfo>,
                         const DenseSet<unsigned, TestDenseSetInfo>,
                         SmallDenseSet<unsigned, 1, TestDenseSetInfo>,
                         SmallDenseSet<unsigned, 4, TestDenseSetInfo>,
                         const SmallDenseSet<unsigned, 4, TestDenseSetInfo>,
                         SmallDenseSet<unsigned, 64, TestDenseSetInfo>>
    DenseSetTestTypes;
TYPED_TEST_CASE(DenseSetTest, DenseSetTestTypes);

TYPED_TEST(DenseSetTest, testInitializerList)
{
  TypeParam set({1, 2, 1, 4});
  EXPECT_EQ(3u, set.getSize());
  EXPECT_EQ(1u, set.count(1));
  EXPECT_EQ(1u, set.count(2));
  EXPECT_EQ(1u, set.count(4));
  EXPECT_EQ(0u, set.count(3));
}

TYPED_TEST(DenseSetTest, testConstIteratorComparison)
{
  TypeParam set({1});
  const TypeParam &cset = set;
  EXPECT_EQ(set.begin(), cset.begin());
  EXPECT_EQ(set.end(), cset.end());
  EXPECT_NE(set.end(), cset.begin());
  EXPECT_NE(set.begin(), cset.end());
}

TYPED_TEST(DenseSetTest, testDefaultConstruction)
{
  typename TypeParam::iterator I, J;
  typename TypeParam::const_iterator CI, CJ;
  EXPECT_EQ(I, J);
  EXPECT_EQ(CI, CJ);
}

TYPED_TEST(DenseSetTest, testEmptyInitializerList)
{
  TypeParam set({});
  EXPECT_EQ(0u, set.getSize());
  EXPECT_EQ(0u, set.count(0));
}

TYPED_TEST(DenseSetTest, testFindAsTest)
{
  auto &set = this->Set;
  // Size tests
  EXPECT_EQ(3u, set.getSize());

  // Normal lookup tests
  EXPECT_EQ(1u, set.count(1));
  EXPECT_EQ(0u, *set.find(0));
  EXPECT_EQ(1u, *set.find(1));
  EXPECT_EQ(2u, *set.find(2));
  EXPECT_TRUE(set.find(3) == set.end());

  // findAs() tests
  EXPECT_EQ(0u, *set.findAs("a"));
  EXPECT_EQ(1u, *set.findAs("b"));
  EXPECT_EQ(2u, *set.findAs("c"));
  EXPECT_TRUE(set.findAs("d") == set.end());
}

// Simple class that counts how many moves and copy happens when growing a map
struct CountCopyAndMove
{
  static int Move;
  static int Copy;
  int Value;
  CountCopyAndMove(int Value) : Value(Value) {}

  CountCopyAndMove(const CountCopyAndMove &RHS) {
    Value = RHS.Value;
    Copy++;
  }
  CountCopyAndMove &operator=(const CountCopyAndMove &RHS) {
    Value = RHS.Value;
    Copy++;
    return *this;
  }
  CountCopyAndMove(CountCopyAndMove &&RHS) {
    Value = RHS.Value;
    Move++;
  }
  CountCopyAndMove &operator=(const CountCopyAndMove &&RHS) {
    Value = RHS.Value;
    Move++;
    return *this;
  }
};
int CountCopyAndMove::Copy = 0;
int CountCopyAndMove::Move = 0;
} // anonymous namespace

namespace polar {
namespace basic {
// Specialization required to insert a CountCopyAndMove into a DenseSet.
template <> struct DenseMapInfo<CountCopyAndMove> {
  static inline CountCopyAndMove getEmptyKey() { return CountCopyAndMove(-1); }
  static inline CountCopyAndMove getTombstoneKey() {
    return CountCopyAndMove(-2);
  }
  static unsigned getHashValue(const CountCopyAndMove &Val) {
    return Val.Value;
  }
  static bool isEqual(const CountCopyAndMove &LHS,
                      const CountCopyAndMove &RHS) {
    return LHS.Value == RHS.Value;
  }
};
}
}

namespace {
// Make sure reserve actually gives us enough buckets to insert N items
// without increasing allocation size.
TEST(DenseSetCustomTest, testReserveTest) {
  // Test a few different size, 48 is *not* a random choice: we need a value
  // that is 2/3 of a power of two to stress the grow() condition, and the power
  // of two has to be at least 64 because of minimum size allocation in the
  // DenseMa. 66 is a value just above the 64 default init.
  for (auto Size : {1, 2, 48, 66}) {
    DenseSet<CountCopyAndMove> Set;
    Set.reserve(Size);
    unsigned MemorySize = Set.getMemorySize();
    CountCopyAndMove::Copy = 0;
    CountCopyAndMove::Move = 0;
    for (int i = 0; i < Size; ++i)
      Set.insert(CountCopyAndMove(i));
    // Check that we didn't grow
    EXPECT_EQ(MemorySize, Set.getMemorySize());
    // Check that move was called the expected number of times
    EXPECT_EQ(Size, CountCopyAndMove::Move);
    // Check that no copy occured
    EXPECT_EQ(0, CountCopyAndMove::Copy);
  }
}
TEST(DenseSetCustomTest, testConstTest) {
  // Test that const pointers work okay for count and find, even when the
  // underlying map is a non-const pointer.
  DenseSet<int *> Map;
  int A;
  int *B = &A;
  const int *C = &A;
  Map.insert(B);
  EXPECT_EQ(Map.count(B), 1u);
  EXPECT_EQ(Map.count(C), 1u);
  EXPECT_NE(Map.find(B), Map.end());
  EXPECT_NE(Map.find(C), Map.end());
}

} // anonymous namespace
