// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/07/12.

#include "polar/utils/Debug.h"
#include "polar/utils/RawOutStream.h".h"
#include "gtest/gtest.h"
#include <string>

using namespace polar::basic;
using namespace polar::utils;

namespace {

#ifdef POLAR_DEBUG
TEST(DebugTest, testBasic)
{
  std::string s1, s2;
  RawStringOutStream os1(s1), os2(s2);
  static const char *DT[] = {"A", "B"};

  polar::utils::sg_debugFlag = true;
  set_current_debug_types(DT, 2);
  DEBUG_WITH_TYPE("A", os1 << "A");
  DEBUG_WITH_TYPE("B", os1 << "B");
  EXPECT_EQ("AB", os1.getStr());

  set_current_debug_type("A");
  DEBUG_WITH_TYPE("A", os2 << "A");
  DEBUG_WITH_TYPE("B", os2 << "B");
  EXPECT_EQ("A", os2.getStr());
}
#endif

} // anonymous namespace
