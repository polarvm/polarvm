// This source file is part of the polarphp.org open source project

// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception

// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors

// Created by softboy on 2018/07/10.

#include "polar/utils/Path.h"
#include "polar/basic/adt/STLExtras.h"
#include "polar/basic/adt/SmallVector.h"
#include "polar/basic/adt/Triple.h"
#include "polar/utils/ConvertUTF.h"
#include "polar/utils/ErrorCode.h"
#include "polar/utils/ErrorHandling.h"
#include "polar/utils/FileSystem.h"
#include "polar/utils/FileUtils.h"
#include "polar/utils/Host.h"
#include "polar/utils/MemoryBuffer.h"
#include "polar/utils/RawOutStream.h"
#include "gtest/gtest.h"

#ifdef POLAR_ON_UNIX
#include <pwd.h>
#include <sys/stat.h>
#endif

using namespace polar::basic;
using namespace polar::utils;
using namespace polar::sys;
using namespace polar::fs;

#define ASSERT_NO_ERROR(x)                                                     \
   if (std::error_code ASSERT_NO_ERROR_ec = x) {                                \
   SmallString<128> MessageStorage;                                           \
   RawSvectorOutStream Message(MessageStorage);                               \
   Message << #x ": did not return errc::success.\n"                          \
   << "error number: " << ASSERT_NO_ERROR_ec.value() << "\n"          \
   << "error message: " << ASSERT_NO_ERROR_ec.message() << "\n";      \
   GTEST_FATAL_FAILURE_(MessageStorage.c_str());                              \
   } else {                                                                     \
   }


namespace {

TEST(PathTest, testWorks)
{
  EXPECT_TRUE(polar::fs::path::is_separator('/'));
  EXPECT_FALSE(path::is_separator('\0'));
  EXPECT_FALSE(path::is_separator('-'));
  EXPECT_FALSE(path::is_separator(' '));

//  EXPECT_TRUE(path::is_separator('\\', path::Style::windows));
//  EXPECT_FALSE(path::is_separator('\\', path::Style::posix));

//#ifdef POLAR_ON_WIN32
//  EXPECT_TRUE(path::is_separator('\\'));
//#else
//  EXPECT_FALSE(path::is_separator('\\'));
//#endif
}

} // anonymous namespace
