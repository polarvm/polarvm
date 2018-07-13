// This source file is part of the polarphp.org open source project

// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception

// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors

// Created by softboy on 2018/07/13.

#include "polar/basic/adt/SmallString.h"
#include "polar/utils/FormattedStream.h"
#include "polar/utils/RawOutStream.h"
#include "gtest/gtest.h"

using namespace polar;
using namespace polar::basic;
using namespace polar::utils;

namespace {

TEST(FormattedRawOutStreamTest, testTell)
{
   // Check offset when underlying stream has buffer contents.
   SmallString<128> A;
   RawSvectorOutStream B(A);
   FormattedRawOutStream C(B);
   char tmp[100] = "";

   for (unsigned i = 0; i != 3; ++i) {
      C.write(tmp, 100);
      EXPECT_EQ(100*(i+1), (unsigned) C.tell());
   }
}

} // anonymous namespace

