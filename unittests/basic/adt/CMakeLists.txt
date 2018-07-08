set(CMAKE_INCLUDE_CURRENT_DIR ON)

polar_add_unittest(PolarBaseLibTests BasicAdtTest
   ApIntTest.cpp
   ApSIntTest.cpp
   ApFloatTest.cpp
   BitVectorTest.cpp
   BumpPtrListTest.cpp
   BreadthFirstIteratorTest.cpp
   EquivalenceClassesTest.cpp
   HashingTest.cpp
   StlExtrasTest.cpp
   ArrayRefTest.cpp
   IntrusiveListBaseTest.cpp
   IntrusiveListNodeBaseTest.cpp
   IntrusiveListNodeTest.cpp
   IntrusiveListIteratorTest.cpp
   IntrusiveListSentienelTest.cpp
   IntrusiveListTest.cpp
   ImmutableMapTest.cpp
   ImmutableSetTest.cpp
   IntEqClassesTest.cpp
   IntervalMapTest.cpp
   SmallVectorTest.cpp
   BitMaskEnumTest.cpp
   FoldingSetTest.cpp
   DenseMapTest.cpp
   DenseSetTest.cpp
   FunctionRefTest.cpp
   StringExtrasTest.cpp
   SmallStringTest.cpp
   SmallPtrSetTest.cpp
   SequenceTest.cpp
   ScopeExitTest.cpp
   StringRefTest.cpp
   StringSwitchTest.cpp
   TripleTest.cpp
   TestGraph.h
   )

