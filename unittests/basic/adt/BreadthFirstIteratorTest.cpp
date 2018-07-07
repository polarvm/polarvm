// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/07/07.

#include "polar/basic/adt/BreadthFirstIterator.h"
#include "TestGraph.h"
#include "gtest/gtest.h"

using namespace polar::basic;

namespace {

TEST(BreadthFristIteratorTest, testBasic)
{
   typedef BreadthFirstIterator<Graph<4>> BFIter;

   Graph<4> G;
   G.addEdge(0, 1);
   G.addEdge(0, 2);
   G.addEdge(1, 3);

   auto It = BFIter::begin(G);
   auto End = BFIter::end(G);
   EXPECT_EQ(It.getLevel(), 0U);
   EXPECT_EQ(*It, G.accessNode(0));
   ++It;
   EXPECT_EQ(It.getLevel(), 1U);
   EXPECT_EQ(*It, G.accessNode(1));
   ++It;
   EXPECT_EQ(It.getLevel(), 1U);
   EXPECT_EQ(*It, G.accessNode(2));
   ++It;
   EXPECT_EQ(It.getLevel(), 2U);
   EXPECT_EQ(*It, G.accessNode(3));
   ++It;
   EXPECT_EQ(It, End);
}

TEST(BreadthFristIteratorTest, testCycle)
{
   typedef BreadthFirstIterator<Graph<4>> BFIter;

   Graph<4> G;
   G.addEdge(0, 1);
   G.addEdge(1, 0);
   G.addEdge(1, 2);
   G.addEdge(2, 1);
   G.addEdge(2, 1);
   G.addEdge(2, 3);
   G.addEdge(3, 2);
   G.addEdge(3, 1);
   G.addEdge(3, 0);

   auto It = BFIter::begin(G);
   auto End = BFIter::end(G);
   EXPECT_EQ(It.getLevel(), 0U);
   EXPECT_EQ(*It, G.accessNode(0));
   ++It;
   EXPECT_EQ(It.getLevel(), 1U);
   EXPECT_EQ(*It, G.accessNode(1));
   ++It;
   EXPECT_EQ(It.getLevel(), 2U);
   EXPECT_EQ(*It, G.accessNode(2));
   ++It;
   EXPECT_EQ(It.getLevel(), 3U);
   EXPECT_EQ(*It, G.accessNode(3));
   ++It;
   EXPECT_EQ(It, End);
}

} // anonymous namespace
