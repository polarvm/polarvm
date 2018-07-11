// This source file is part of the polarphp.org open source project

// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception

// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors

// Created by softboy on 2018/07/06.

#include "polar/basic/adt/SmallVector.h"
#include "polar/basic/adt/ArrayRef.h"
#include "gtest/gtest.h"
#include <list>
#include <stdarg.h>

namespace {

using polar::basic::SmallVector;
using polar::basic::ArrayRef;
using polar::basic::SmallVectorImpl;
using polar::basic::make_array_ref;

/// A helper class that counts the total number of constructor and
/// destructor calls.
class Constructable {
private:
   static int numConstructorCalls;
   static int numMoveConstructorCalls;
   static int numCopyConstructorCalls;
   static int numDestructorCalls;
   static int numAssignmentCalls;
   static int numMoveAssignmentCalls;
   static int numCopyAssignmentCalls;

   bool constructed;
   int value;

public:
   Constructable() : constructed(true), value(0)
   {
      ++numConstructorCalls;
   }

   Constructable(int val) : constructed(true), value(val)
   {
      ++numConstructorCalls;
   }

   Constructable(const Constructable & src) : constructed(true)
   {
      value = src.value;
      ++numConstructorCalls;
      ++numCopyConstructorCalls;
   }

   Constructable(Constructable && src) : constructed(true)
   {
      value = src.value;
      ++numConstructorCalls;
      ++numMoveConstructorCalls;
   }

   ~Constructable()
   {
      EXPECT_TRUE(constructed);
      ++numDestructorCalls;
      constructed = false;
   }

   Constructable & operator=(const Constructable & src)
   {
      EXPECT_TRUE(constructed);
      value = src.value;
      ++numAssignmentCalls;
      ++numCopyAssignmentCalls;
      return *this;
   }

   Constructable & operator=(Constructable && src)
   {
      EXPECT_TRUE(constructed);
      value = src.value;
      ++numAssignmentCalls;
      ++numMoveAssignmentCalls;
      return *this;
   }

   int getValue() const
   {
      return abs(value);
   }

   static void reset()
   {
      numConstructorCalls = 0;
      numMoveConstructorCalls = 0;
      numCopyConstructorCalls = 0;
      numDestructorCalls = 0;
      numAssignmentCalls = 0;
      numMoveAssignmentCalls = 0;
      numCopyAssignmentCalls = 0;
   }

   static int getNumConstructorCalls()
   {
      return numConstructorCalls;
   }

   static int getNumMoveConstructorCalls()
   {
      return numMoveConstructorCalls;
   }

   static int getNumCopyConstructorCalls()
   {
      return numCopyConstructorCalls;
   }

   static int getNumDestructorCalls()
   {
      return numDestructorCalls;
   }

   static int getNumAssignmentCalls()
   {
      return numAssignmentCalls;
   }

   static int getNumMoveAssignmentCalls()
   {
      return numMoveAssignmentCalls;
   }

   static int getNumCopyAssignmentCalls()
   {
      return numCopyAssignmentCalls;
   }

   friend bool operator==(const Constructable & c0, const Constructable & c1)
   {
      return c0.getValue() == c1.getValue();
   }

   friend bool POLAR_ATTRIBUTE_UNUSED
   operator!=(const Constructable & c0, const Constructable & c1)
   {
      return c0.getValue() != c1.getValue();
   }
};

int Constructable::numConstructorCalls;
int Constructable::numCopyConstructorCalls;
int Constructable::numMoveConstructorCalls;
int Constructable::numDestructorCalls;
int Constructable::numAssignmentCalls;
int Constructable::numCopyAssignmentCalls;
int Constructable::numMoveAssignmentCalls;

struct NonCopyable
{
   NonCopyable() {}
   NonCopyable(NonCopyable &&) {}
   NonCopyable &operator=(NonCopyable &&) { return *this; }
private:
   NonCopyable(const NonCopyable &) = delete;
   NonCopyable &operator=(const NonCopyable &) = delete;
};

POLAR_ATTRIBUTE_USED void CompileTest()
{
   SmallVector<NonCopyable, 0> V;
   V.resize(42);
}

class SmallVectorTestBase : public testing::Test
{
protected:
   void SetUp() override { Constructable::reset(); }

   template <typename VectorT>
   void assertEmpty(VectorT & v) {
      // Size tests
      EXPECT_EQ(0u, v.getSize());
      EXPECT_TRUE(v.empty());

      // Iterator tests
      EXPECT_TRUE(v.begin() == v.end());
   }

   // Assert that v contains the specified values, in order.
   template <typename VectorT>
   void assertValuesInOrder(VectorT & v, size_t size, ...) {
      EXPECT_EQ(size, v.getSize());

      va_list ap;
      va_start(ap, size);
      for (size_t i = 0; i < size; ++i) {
         int value = va_arg(ap, int);
         EXPECT_EQ(value, v[i].getValue());
      }

      va_end(ap);
   }

   // Generate a sequence of values to initialize the vector.
   template <typename VectorT>
   void makeSequence(VectorT & v, int start, int end) {
      for (int i = start; i <= end; ++i) {
         v.push_back(Constructable(i));
      }
   }
};

// Test fixture class
template <typename VectorT>
class SmallVectorTest : public SmallVectorTestBase
{
protected:
   VectorT theVector;
   VectorT otherVector;
};


typedef ::testing::Types<SmallVector<Constructable, 0>,
SmallVector<Constructable, 1>,
SmallVector<Constructable, 2>,
SmallVector<Constructable, 4>,
SmallVector<Constructable, 5>
> SmallVectorTestTypes;
TYPED_TEST_CASE(SmallVectorTest, SmallVectorTestTypes);

// Clear test.
TYPED_TEST(SmallVectorTest, testClearTest)
{
   SCOPED_TRACE("ClearTest");

   this->theVector.reserve(2);
   this->makeSequence(this->theVector, 1, 2);
   this->theVector.clear();

   this->assertEmpty(this->theVector);
   EXPECT_EQ(4, Constructable::getNumConstructorCalls());
   EXPECT_EQ(4, Constructable::getNumDestructorCalls());
}

// Resize smaller test.
TYPED_TEST(SmallVectorTest, testResizeShrinkTest)
{
   SCOPED_TRACE("ResizeShrinkTest");

   this->theVector.reserve(3);
   this->makeSequence(this->theVector, 1, 3);
   this->theVector.resize(1);

   this->assertValuesInOrder(this->theVector, 1u, 1);
   EXPECT_EQ(6, Constructable::getNumConstructorCalls());
   EXPECT_EQ(5, Constructable::getNumDestructorCalls());
}

// Resize bigger test.
TYPED_TEST(SmallVectorTest, testResizeGrowTest)
{
   SCOPED_TRACE("ResizeGrowTest");

   this->theVector.resize(2);

   EXPECT_EQ(2, Constructable::getNumConstructorCalls());
   EXPECT_EQ(0, Constructable::getNumDestructorCalls());
   EXPECT_EQ(2u, this->theVector.getSize());
}

TYPED_TEST(SmallVectorTest, testResizeWithElementsTest)
{
   this->theVector.resize(2);

   Constructable::reset();

   this->theVector.resize(4);

   size_t Ctors = Constructable::getNumConstructorCalls();
   EXPECT_TRUE(Ctors == 2 || Ctors == 4);
   size_t MoveCtors = Constructable::getNumMoveConstructorCalls();
   EXPECT_TRUE(MoveCtors == 0 || MoveCtors == 2);
   size_t Dtors = Constructable::getNumDestructorCalls();
   EXPECT_TRUE(Dtors == 0 || Dtors == 2);
}

// Resize with fill value.
TYPED_TEST(SmallVectorTest, testResizeFillTest)
{
   SCOPED_TRACE("ResizeFillTest");

   this->theVector.resize(3, Constructable(77));
   this->assertValuesInOrder(this->theVector, 3u, 77, 77, 77);
}

// Overflow past fixed size.
TYPED_TEST(SmallVectorTest, testOverflowTest)
{
   SCOPED_TRACE("OverflowTest");

   // Push more elements than the fixed size.
   this->makeSequence(this->theVector, 1, 10);

   // Test size and values.
   EXPECT_EQ(10u, this->theVector.getSize());
   for (int i = 0; i < 10; ++i) {
      EXPECT_EQ(i+1, this->theVector[i].getValue());
   }

   // Now resize back to fixed size.
   this->theVector.resize(1);

   this->assertValuesInOrder(this->theVector, 1u, 1);
}

// Iteration tests.
TYPED_TEST(SmallVectorTest, testIterationTest)
{
   this->makeSequence(this->theVector, 1, 2);

   // Forward Iteration
   typename TypeParam::iterator it = this->theVector.begin();
   EXPECT_TRUE(*it == this->theVector.front());
   EXPECT_TRUE(*it == this->theVector[0]);
   EXPECT_EQ(1, it->getValue());
   ++it;
   EXPECT_TRUE(*it == this->theVector[1]);
   EXPECT_TRUE(*it == this->theVector.back());
   EXPECT_EQ(2, it->getValue());
   ++it;
   EXPECT_TRUE(it == this->theVector.end());
   --it;
   EXPECT_TRUE(*it == this->theVector[1]);
   EXPECT_EQ(2, it->getValue());
   --it;
   EXPECT_TRUE(*it == this->theVector[0]);
   EXPECT_EQ(1, it->getValue());

   // Reverse Iteration
   typename TypeParam::reverse_iterator rit = this->theVector.rbegin();
   EXPECT_TRUE(*rit == this->theVector[1]);
   EXPECT_EQ(2, rit->getValue());
   ++rit;
   EXPECT_TRUE(*rit == this->theVector[0]);
   EXPECT_EQ(1, rit->getValue());
   ++rit;
   EXPECT_TRUE(rit == this->theVector.rend());
   --rit;
   EXPECT_TRUE(*rit == this->theVector[0]);
   EXPECT_EQ(1, rit->getValue());
   --rit;
   EXPECT_TRUE(*rit == this->theVector[1]);
   EXPECT_EQ(2, rit->getValue());
}

// Swap test.
TYPED_TEST(SmallVectorTest, testSwapTest)
{
   SCOPED_TRACE("SwapTest");

   this->makeSequence(this->theVector, 1, 2);
   std::swap(this->theVector, this->otherVector);

   this->assertEmpty(this->theVector);
   this->assertValuesInOrder(this->otherVector, 2u, 1, 2);
}

// Append test
TYPED_TEST(SmallVectorTest, testAppendTest)
{
   SCOPED_TRACE("AppendTest");

   this->makeSequence(this->otherVector, 2, 3);

   this->theVector.push_back(Constructable(1));
   this->theVector.append(this->otherVector.begin(), this->otherVector.end());

   this->assertValuesInOrder(this->theVector, 3u, 1, 2, 3);
}

// Append repeated test
TYPED_TEST(SmallVectorTest, testAppendRepeatedTest)
{
   SCOPED_TRACE("AppendRepeatedTest");

   this->theVector.push_back(Constructable(1));
   this->theVector.append(2, Constructable(77));
   this->assertValuesInOrder(this->theVector, 3u, 1, 77, 77);
}

// Append test
TYPED_TEST(SmallVectorTest, testAppendNonIterTest)
{

   SCOPED_TRACE("AppendRepeatedTest");

   this->theVector.push_back(Constructable(1));
   this->theVector.append(2, 7);
   this->assertValuesInOrder(this->theVector, 3u, 1, 7, 7);
}

struct output_iterator
{
   typedef std::output_iterator_tag iterator_category;
   typedef int value_type;
   typedef int difference_type;
   typedef value_type *pointer;
   typedef value_type &reference;
   operator int() { return 2; }
   operator Constructable() { return 7; }
};

TYPED_TEST(SmallVectorTest, testAppendRepeatedNonForwardIterator)
{
   SCOPED_TRACE("AppendRepeatedTest");

   this->theVector.push_back(Constructable(1));
   this->theVector.append(output_iterator(), output_iterator());
   this->assertValuesInOrder(this->theVector, 3u, 1, 7, 7);
}

// Assign test
TYPED_TEST(SmallVectorTest, testAssignTest)
{
   SCOPED_TRACE("AssignTest");

   this->theVector.push_back(Constructable(1));
   this->theVector.assign(2, Constructable(77));
   this->assertValuesInOrder(this->theVector, 2u, 77, 77);
}

// Assign test
TYPED_TEST(SmallVectorTest, testAssignRangeTest)
{
   SCOPED_TRACE("AssignTest");

   this->theVector.push_back(Constructable(1));
   int arr[] = {1, 2, 3};
   this->theVector.assign(std::begin(arr), std::end(arr));
   this->assertValuesInOrder(this->theVector, 3u, 1, 2, 3);
}

// Assign test
TYPED_TEST(SmallVectorTest, testAssignNonIterTest)
{
   SCOPED_TRACE("AssignTest");

   this->theVector.push_back(Constructable(1));
   this->theVector.assign(2, 7);
   this->assertValuesInOrder(this->theVector, 2u, 7, 7);
}

// Move-assign test
TYPED_TEST(SmallVectorTest, testMoveAssignTest)
{
   SCOPED_TRACE("MoveAssignTest");

   // Set up our vector with a single element, but enough capacity for 4.
   this->theVector.reserve(4);
   this->theVector.push_back(Constructable(1));

   // Set up the other vector with 2 elements.
   this->otherVector.push_back(Constructable(2));
   this->otherVector.push_back(Constructable(3));

   // Move-assign from the other vector.
   this->theVector = std::move(this->otherVector);

   // Make sure we have the right result.
   this->assertValuesInOrder(this->theVector, 2u, 2, 3);

   // Make sure the # of constructor/destructor calls line up. There
   // are two live objects after clearing the other vector.
   this->otherVector.clear();
   EXPECT_EQ(Constructable::getNumConstructorCalls()-2,
             Constructable::getNumDestructorCalls());

   // There shouldn't be any live objects any more.
   this->theVector.clear();
   EXPECT_EQ(Constructable::getNumConstructorCalls(),
             Constructable::getNumDestructorCalls());
}

// Erase a single element
TYPED_TEST(SmallVectorTest, testEraseTest)
{
   SCOPED_TRACE("EraseTest");

   this->makeSequence(this->theVector, 1, 3);
   const auto &theConstVector = this->theVector;
   this->theVector.erase(theConstVector.begin());
   this->assertValuesInOrder(this->theVector, 2u, 2, 3);
}

// Erase a range of elements
TYPED_TEST(SmallVectorTest, testEraseRangeTest)
{
   SCOPED_TRACE("EraseRangeTest");

   this->makeSequence(this->theVector, 1, 3);
   const auto &theConstVector = this->theVector;
   this->theVector.erase(theConstVector.begin(), theConstVector.begin() + 2);
   this->assertValuesInOrder(this->theVector, 1u, 3);
}

// Insert a single element.
TYPED_TEST(SmallVectorTest, testInsertTest)
{
   SCOPED_TRACE("InsertTest");

   this->makeSequence(this->theVector, 1, 3);
   typename TypeParam::iterator I =
         this->theVector.insert(this->theVector.begin() + 1, Constructable(77));
   EXPECT_EQ(this->theVector.begin() + 1, I);
   this->assertValuesInOrder(this->theVector, 4u, 1, 77, 2, 3);
}

// Insert a copy of a single element.
TYPED_TEST(SmallVectorTest, testInsertCopy)
{
   SCOPED_TRACE("InsertTest");

   this->makeSequence(this->theVector, 1, 3);
   Constructable C(77);
   typename TypeParam::iterator I =
         this->theVector.insert(this->theVector.begin() + 1, C);
   EXPECT_EQ(this->theVector.begin() + 1, I);
   this->assertValuesInOrder(this->theVector, 4u, 1, 77, 2, 3);
}

// Insert repeated elements.
TYPED_TEST(SmallVectorTest, testInsertRepeatedTest)
{
   SCOPED_TRACE("InsertRepeatedTest");

   this->makeSequence(this->theVector, 1, 4);
   Constructable::reset();
   auto I =
         this->theVector.insert(this->theVector.begin() + 1, 2, Constructable(16));
   // Move construct the top element into newly allocated space, and optionally
   // reallocate the whole buffer, move constructing into it.
   // FIXME: This is inefficient, we shouldn't move things into newly allocated
   // space, then move them up/around, there should only be 2 or 4 move
   // constructions here.
   EXPECT_TRUE(Constructable::getNumMoveConstructorCalls() == 2 ||
               Constructable::getNumMoveConstructorCalls() == 6);
   // Move assign the next two to shift them up and make a gap.
   EXPECT_EQ(1, Constructable::getNumMoveAssignmentCalls());
   // Copy construct the two new elements from the parameter.
   EXPECT_EQ(2, Constructable::getNumCopyAssignmentCalls());
   // All without any copy construction.
   EXPECT_EQ(0, Constructable::getNumCopyConstructorCalls());
   EXPECT_EQ(this->theVector.begin() + 1, I);
   this->assertValuesInOrder(this->theVector, 6u, 1, 16, 16, 2, 3, 4);
}

TYPED_TEST(SmallVectorTest, testInsertRepeatedNonIterTest)
{
   SCOPED_TRACE("InsertRepeatedTest");

   this->makeSequence(this->theVector, 1, 4);
   Constructable::reset();
   auto I = this->theVector.insert(this->theVector.begin() + 1, 2, 7);
   EXPECT_EQ(this->theVector.begin() + 1, I);
   this->assertValuesInOrder(this->theVector, 6u, 1, 7, 7, 2, 3, 4);
}

TYPED_TEST(SmallVectorTest, testInsertRepeatedAtEndTest)
{
   SCOPED_TRACE("InsertRepeatedTest");

   this->makeSequence(this->theVector, 1, 4);
   Constructable::reset();
   auto I = this->theVector.insert(this->theVector.end(), 2, Constructable(16));
   // Just copy construct them into newly allocated space
   EXPECT_EQ(2, Constructable::getNumCopyConstructorCalls());
   // Move everything across if reallocation is needed.
   EXPECT_TRUE(Constructable::getNumMoveConstructorCalls() == 0 ||
               Constructable::getNumMoveConstructorCalls() == 4);
   // Without ever moving or copying anything else.
   EXPECT_EQ(0, Constructable::getNumCopyAssignmentCalls());
   EXPECT_EQ(0, Constructable::getNumMoveAssignmentCalls());

   EXPECT_EQ(this->theVector.begin() + 4, I);
   this->assertValuesInOrder(this->theVector, 6u, 1, 2, 3, 4, 16, 16);
}

TYPED_TEST(SmallVectorTest, testInsertRepeatedEmptyTest)
{
   SCOPED_TRACE("InsertRepeatedTest");

   this->makeSequence(this->theVector, 10, 15);

   // Empty insert.
   EXPECT_EQ(this->theVector.end(),
             this->theVector.insert(this->theVector.end(),
                                    0, Constructable(42)));
   EXPECT_EQ(this->theVector.begin() + 1,
             this->theVector.insert(this->theVector.begin() + 1,
                                    0, Constructable(42)));
}

// Insert range.
TYPED_TEST(SmallVectorTest, testInsertRangeTest)
{
   SCOPED_TRACE("InsertRangeTest");

   Constructable Arr[3] =
   { Constructable(77), Constructable(77), Constructable(77) };

   this->makeSequence(this->theVector, 1, 3);
   Constructable::reset();
   auto I = this->theVector.insert(this->theVector.begin() + 1, Arr, Arr + 3);
   // Move construct the top 3 elements into newly allocated space.
   // Possibly move the whole sequence into new space first.
   // FIXME: This is inefficient, we shouldn't move things into newly allocated
   // space, then move them up/around, there should only be 2 or 3 move
   // constructions here.
   EXPECT_TRUE(Constructable::getNumMoveConstructorCalls() == 2 ||
               Constructable::getNumMoveConstructorCalls() == 5);
   // Copy assign the lower 2 new elements into existing space.
   EXPECT_EQ(2, Constructable::getNumCopyAssignmentCalls());
   // Copy construct the third element into newly allocated space.
   EXPECT_EQ(1, Constructable::getNumCopyConstructorCalls());
   EXPECT_EQ(this->theVector.begin() + 1, I);
   this->assertValuesInOrder(this->theVector, 6u, 1, 77, 77, 77, 2, 3);
}


TYPED_TEST(SmallVectorTest, testInsertRangeAtEndTest)
{
   SCOPED_TRACE("InsertRangeTest");

   Constructable Arr[3] =
   { Constructable(77), Constructable(77), Constructable(77) };

   this->makeSequence(this->theVector, 1, 3);

   // Insert at end.
   Constructable::reset();
   auto I = this->theVector.insert(this->theVector.end(), Arr, Arr+3);
   // Copy construct the 3 elements into new space at the top.
   EXPECT_EQ(3, Constructable::getNumCopyConstructorCalls());
   // Don't copy/move anything else.
   EXPECT_EQ(0, Constructable::getNumCopyAssignmentCalls());
   // Reallocation might occur, causing all elements to be moved into the new
   // buffer.
   EXPECT_TRUE(Constructable::getNumMoveConstructorCalls() == 0 ||
               Constructable::getNumMoveConstructorCalls() == 3);
   EXPECT_EQ(0, Constructable::getNumMoveAssignmentCalls());
   EXPECT_EQ(this->theVector.begin() + 3, I);
   this->assertValuesInOrder(this->theVector, 6u,
                             1, 2, 3, 77, 77, 77);
}

TYPED_TEST(SmallVectorTest, testInsertEmptyRangeTest)
{
   SCOPED_TRACE("InsertRangeTest");

   this->makeSequence(this->theVector, 1, 3);

   // Empty insert.
   EXPECT_EQ(this->theVector.end(),
             this->theVector.insert(this->theVector.end(),
                                    this->theVector.begin(),
                                    this->theVector.begin()));
   EXPECT_EQ(this->theVector.begin() + 1,
             this->theVector.insert(this->theVector.begin() + 1,
                                    this->theVector.begin(),
                                    this->theVector.begin()));
}

// Comparison tests.
TYPED_TEST(SmallVectorTest, testComparisonTest)
{
   SCOPED_TRACE("ComparisonTest");

   this->makeSequence(this->theVector, 1, 3);
   this->makeSequence(this->otherVector, 1, 3);

   EXPECT_TRUE(this->theVector == this->otherVector);
   EXPECT_FALSE(this->theVector != this->otherVector);

   this->otherVector.clear();
   this->makeSequence(this->otherVector, 2, 4);

   EXPECT_FALSE(this->theVector == this->otherVector);
   EXPECT_TRUE(this->theVector != this->otherVector);
}

// Constant vector tests.
TYPED_TEST(SmallVectorTest, testConstVectorTest)
{
   const TypeParam constVector;

   EXPECT_EQ(0u, constVector.getSize());
   EXPECT_TRUE(constVector.empty());
   EXPECT_TRUE(constVector.begin() == constVector.end());
}

// Direct array access.
TYPED_TEST(SmallVectorTest, testDirectVectorTest)
{
   EXPECT_EQ(0u, this->theVector.getSize());
   this->theVector.reserve(4);
   EXPECT_LE(4u, this->theVector.getCapacity());
   EXPECT_EQ(0, Constructable::getNumConstructorCalls());
   this->theVector.push_back(1);
   this->theVector.push_back(2);
   this->theVector.push_back(3);
   this->theVector.push_back(4);
   EXPECT_EQ(4u, this->theVector.getSize());
   EXPECT_EQ(8, Constructable::getNumConstructorCalls());
   EXPECT_EQ(1, this->theVector[0].getValue());
   EXPECT_EQ(2, this->theVector[1].getValue());
   EXPECT_EQ(3, this->theVector[2].getValue());
   EXPECT_EQ(4, this->theVector[3].getValue());
}

TYPED_TEST(SmallVectorTest, testIteratorTest)
{
   std::list<int> L;
   this->theVector.insert(this->theVector.end(), L.begin(), L.end());
}

template <typename InvalidType> class DualSmallVectorsTest;

template <typename VectorT1, typename VectorT2>
class DualSmallVectorsTest<std::pair<VectorT1, VectorT2>> : public SmallVectorTestBase {
protected:
   VectorT1 theVector;
   VectorT2 otherVector;

   template <typename T, unsigned N>
   static unsigned NumBuiltinElts(const SmallVector<T, N>&) { return N; }
};

typedef ::testing::Types<
// Small mode -> Small mode.
std::pair<SmallVector<Constructable, 4>, SmallVector<Constructable, 4>>,
// Small mode -> Big mode.
std::pair<SmallVector<Constructable, 4>, SmallVector<Constructable, 2>>,
// Big mode -> Small mode.
std::pair<SmallVector<Constructable, 2>, SmallVector<Constructable, 4>>,
// Big mode -> Big mode.
std::pair<SmallVector<Constructable, 2>, SmallVector<Constructable, 2>>
> DualSmallVectorTestTypes;

TYPED_TEST_CASE(DualSmallVectorsTest, DualSmallVectorTestTypes);

TYPED_TEST(DualSmallVectorsTest, testMoveAssignment)
{
   SCOPED_TRACE("MoveAssignTest-DualVectorTypes");

   // Set up our vector with four elements.
   for (unsigned I = 0; I < 4; ++I)
      this->otherVector.push_back(Constructable(I));

   const Constructable *OrigDataPtr = this->otherVector.getData();

   // Move-assign from the other vector.
   this->theVector =
         std::move(static_cast<SmallVectorImpl<Constructable>&>(this->otherVector));

   // Make sure we have the right result.
   this->assertValuesInOrder(this->theVector, 4u, 0, 1, 2, 3);

   // Make sure the # of constructor/destructor calls line up. There
   // are two live objects after clearing the other vector.
   this->otherVector.clear();
   EXPECT_EQ(Constructable::getNumConstructorCalls()-4,
             Constructable::getNumDestructorCalls());

   // If the source vector (otherVector) was in small-mode, assert that we just
   // moved the data pointer over.
   EXPECT_TRUE(this->NumBuiltinElts(this->otherVector) == 4 ||
               this->theVector.getData() == OrigDataPtr);

   // There shouldn't be any live objects any more.
   this->theVector.clear();
   EXPECT_EQ(Constructable::getNumConstructorCalls(),
             Constructable::getNumDestructorCalls());

   // We shouldn't have copied anything in this whole process.
   EXPECT_EQ(Constructable::getNumCopyConstructorCalls(), 0);
}

struct notassignable
{
   int &x;
   notassignable(int &x) : x(x) {}
};

TEST(SmallVectorCustomTest, testNoAssignTest)
{
   int x = 0;
   SmallVector<notassignable, 2> vec;
   vec.push_back(notassignable(x));
   x = 42;
   EXPECT_EQ(42, vec.popBackValue().x);
}

struct MovedFrom
{
   bool hasValue;
   MovedFrom() : hasValue(true) {
   }
   MovedFrom(MovedFrom&& m) : hasValue(m.hasValue) {
      m.hasValue = false;
   }
   MovedFrom &operator=(MovedFrom&& m) {
      hasValue = m.hasValue;
      m.hasValue = false;
      return *this;
   }
};

TEST(SmallVectorTest, testMidInsert)
{
   SmallVector<MovedFrom, 3> v;
   v.push_back(MovedFrom());
   v.insert(v.begin(), MovedFrom());
   for (MovedFrom &m : v)
      EXPECT_TRUE(m.hasValue);
}

enum EmplaceableArgState
{
   EAS_Defaulted,
   EAS_Arg,
   EAS_LValue,
   EAS_RValue,
   EAS_Failure
};
template <int I> struct EmplaceableArg
{
   EmplaceableArgState State;
   EmplaceableArg() : State(EAS_Defaulted) {}
   EmplaceableArg(EmplaceableArg &&X)
      : State(X.State == EAS_Arg ? EAS_RValue : EAS_Failure) {}
   EmplaceableArg(EmplaceableArg &X)
      : State(X.State == EAS_Arg ? EAS_LValue : EAS_Failure) {}

   explicit EmplaceableArg(bool) : State(EAS_Arg) {}

private:
   EmplaceableArg &operator=(EmplaceableArg &&) = delete;
   EmplaceableArg &operator=(const EmplaceableArg &) = delete;
};

enum EmplaceableState { ES_Emplaced, ES_Moved };
struct Emplaceable {
   EmplaceableArg<0> A0;
   EmplaceableArg<1> A1;
   EmplaceableArg<2> A2;
   EmplaceableArg<3> A3;
   EmplaceableState State;

   Emplaceable() : State(ES_Emplaced) {}

   template <class A0Ty>
   explicit Emplaceable(A0Ty &&A0)
      : A0(std::forward<A0Ty>(A0)), State(ES_Emplaced) {}

   template <class A0Ty, class A1Ty>
   Emplaceable(A0Ty &&A0, A1Ty &&A1)
      : A0(std::forward<A0Ty>(A0)), A1(std::forward<A1Ty>(A1)),
        State(ES_Emplaced) {}

   template <class A0Ty, class A1Ty, class A2Ty>
   Emplaceable(A0Ty &&A0, A1Ty &&A1, A2Ty &&A2)
      : A0(std::forward<A0Ty>(A0)), A1(std::forward<A1Ty>(A1)),
        A2(std::forward<A2Ty>(A2)), State(ES_Emplaced) {}

   template <class A0Ty, class A1Ty, class A2Ty, class A3Ty>
   Emplaceable(A0Ty &&A0, A1Ty &&A1, A2Ty &&A2, A3Ty &&A3)
      : A0(std::forward<A0Ty>(A0)), A1(std::forward<A1Ty>(A1)),
        A2(std::forward<A2Ty>(A2)), A3(std::forward<A3Ty>(A3)),
        State(ES_Emplaced) {}

   Emplaceable(Emplaceable &&) : State(ES_Moved) {}
   Emplaceable &operator=(Emplaceable &&) {
      State = ES_Moved;
      return *this;
   }

private:
   Emplaceable(const Emplaceable &) = delete;
   Emplaceable &operator=(const Emplaceable &) = delete;
};

TEST(SmallVectorTest, testEmplaceBack)
{
   EmplaceableArg<0> A0(true);
   EmplaceableArg<1> A1(true);
   EmplaceableArg<2> A2(true);
   EmplaceableArg<3> A3(true);
   {
      SmallVector<Emplaceable, 3> V;
      V.emplace_back();
      EXPECT_TRUE(V.getSize() == 1);
      EXPECT_TRUE(V.back().State == ES_Emplaced);
      EXPECT_TRUE(V.back().A0.State == EAS_Defaulted);
      EXPECT_TRUE(V.back().A1.State == EAS_Defaulted);
      EXPECT_TRUE(V.back().A2.State == EAS_Defaulted);
      EXPECT_TRUE(V.back().A3.State == EAS_Defaulted);
   }
   {
      SmallVector<Emplaceable, 3> V;
      V.emplace_back(std::move(A0));
      EXPECT_TRUE(V.getSize() == 1);
      EXPECT_TRUE(V.back().State == ES_Emplaced);
      EXPECT_TRUE(V.back().A0.State == EAS_RValue);
      EXPECT_TRUE(V.back().A1.State == EAS_Defaulted);
      EXPECT_TRUE(V.back().A2.State == EAS_Defaulted);
      EXPECT_TRUE(V.back().A3.State == EAS_Defaulted);
   }
   {
      SmallVector<Emplaceable, 3> V;
      V.emplace_back(A0);
      EXPECT_TRUE(V.getSize() == 1);
      EXPECT_TRUE(V.back().State == ES_Emplaced);
      EXPECT_TRUE(V.back().A0.State == EAS_LValue);
      EXPECT_TRUE(V.back().A1.State == EAS_Defaulted);
      EXPECT_TRUE(V.back().A2.State == EAS_Defaulted);
      EXPECT_TRUE(V.back().A3.State == EAS_Defaulted);
   }
   {
      SmallVector<Emplaceable, 3> V;
      V.emplace_back(A0, A1);
      EXPECT_TRUE(V.getSize() == 1);
      EXPECT_TRUE(V.back().State == ES_Emplaced);
      EXPECT_TRUE(V.back().A0.State == EAS_LValue);
      EXPECT_TRUE(V.back().A1.State == EAS_LValue);
      EXPECT_TRUE(V.back().A2.State == EAS_Defaulted);
      EXPECT_TRUE(V.back().A3.State == EAS_Defaulted);
   }
   {
      SmallVector<Emplaceable, 3> V;
      V.emplace_back(std::move(A0), std::move(A1));
      EXPECT_TRUE(V.getSize() == 1);
      EXPECT_TRUE(V.back().State == ES_Emplaced);
      EXPECT_TRUE(V.back().A0.State == EAS_RValue);
      EXPECT_TRUE(V.back().A1.State == EAS_RValue);
      EXPECT_TRUE(V.back().A2.State == EAS_Defaulted);
      EXPECT_TRUE(V.back().A3.State == EAS_Defaulted);
   }
   {
      SmallVector<Emplaceable, 3> V;
      V.emplaceBack(std::move(A0), A1, std::move(A2), A3);
      EXPECT_TRUE(V.getSize() == 1);
      EXPECT_TRUE(V.back().State == ES_Emplaced);
      EXPECT_TRUE(V.back().A0.State == EAS_RValue);
      EXPECT_TRUE(V.back().A1.State == EAS_LValue);
      EXPECT_TRUE(V.back().A2.State == EAS_RValue);
      EXPECT_TRUE(V.back().A3.State == EAS_LValue);
   }
   {
      SmallVector<int, 1> V;
      V.emplaceBack();
      V.emplaceBack(42);
      EXPECT_EQ(2U, V.getSize());
      EXPECT_EQ(0, V[0]);
      EXPECT_EQ(42, V[1]);
   }
}

TEST(SmallVectorTest, testInitializerList)
{
   SmallVector<int, 2> V1 = {};
   EXPECT_TRUE(V1.empty());
   V1 = {0, 0};
   EXPECT_TRUE(make_array_ref(V1).equals({0, 0}));
   V1 = {-1, -1};
   EXPECT_TRUE(make_array_ref(V1).equals({-1, -1}));

   SmallVector<int, 2> V2 = {1, 2, 3, 4};
   EXPECT_TRUE(make_array_ref(V2).equals({1, 2, 3, 4}));
   V2.assign({4});
   EXPECT_TRUE(make_array_ref(V2).equals({4}));
   V2.append({3, 2});
   EXPECT_TRUE(make_array_ref(V2).equals({4, 3, 2}));
   V2.insert(V2.begin() + 1, 5);
   EXPECT_TRUE(make_array_ref(V2).equals({4, 5, 3, 2}));
}

} // anonymous namespace
