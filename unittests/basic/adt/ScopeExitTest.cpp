// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/07/06.

#include "polar/basic/adt/ScopeExit.h"
#include "gtest/gtest.h"

using namespace polar::basic;

namespace {

TEST(ScopeExitTest, testBasic)
{
  struct Callable {
    bool &Called;
    Callable(bool &Called) : Called(Called) {}
    Callable(Callable &&RHS) : Called(RHS.Called) {}
    void operator()() { Called = true; }
  };
  bool Called = false;
  {
    auto g = make_scope_exit(Callable(Called));
    EXPECT_FALSE(Called);
  }
  EXPECT_TRUE(Called);
}

} // end anonymous namespace
