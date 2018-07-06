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

#include "polar/basic/adt/StringExtras.h"
#include "polar/utils/RawOutStream.h"
#include "gtest/gtest.h"

using namespace polar::basic;

namespace {

TEST(StringExtrasTest, Join)
{
  std::vector<std::string> Items;
  EXPECT_EQ("", join(Items.begin(), Items.end(), " <sep> "));

  Items = {"foo"};
  EXPECT_EQ("foo", join(Items.begin(), Items.end(), " <sep> "));

  Items = {"foo", "bar"};
  EXPECT_EQ("foo <sep> bar", join(Items.begin(), Items.end(), " <sep> "));

  Items = {"foo", "bar", "baz"};
  EXPECT_EQ("foo <sep> bar <sep> baz",
            join(Items.begin(), Items.end(), " <sep> "));
}

TEST(StringExtrasTest, JoinItems)
{
  const char *Foo = "foo";
  std::string Bar = "bar";
  StringRef Baz = "baz";
  char X = 'x';

  EXPECT_EQ("", join_items(" <sep> "));
  EXPECT_EQ("", join_items('/'));

  EXPECT_EQ("foo", join_items(" <sep> ", Foo));
  EXPECT_EQ("foo", join_items('/', Foo));

  EXPECT_EQ("foo <sep> bar", join_items(" <sep> ", Foo, Bar));
  EXPECT_EQ("foo/bar", join_items('/', Foo, Bar));

  EXPECT_EQ("foo <sep> bar <sep> baz", join_items(" <sep> ", Foo, Bar, Baz));
  EXPECT_EQ("foo/bar/baz", join_items('/', Foo, Bar, Baz));

  EXPECT_EQ("foo <sep> bar <sep> baz <sep> x",
            join_items(" <sep> ", Foo, Bar, Baz, X));

  EXPECT_EQ("foo/bar/baz/x", join_items('/', Foo, Bar, Baz, X));
}

TEST(StringExtrasTest, ToAndFromHex)
{
  std::vector<uint8_t> OddBytes = {0x5, 0xBD, 0x0D, 0x3E, 0xCD};
  std::string OddStr = "05BD0D3ECD";
  StringRef OddData(reinterpret_cast<const char *>(OddBytes.data()),
                    OddBytes.size());
  EXPECT_EQ(OddStr, to_hex(OddData));
  EXPECT_EQ(OddData, from_hex(StringRef(OddStr).dropFront()));

  std::vector<uint8_t> EvenBytes = {0xA5, 0xBD, 0x0D, 0x3E, 0xCD};
  std::string EvenStr = "A5BD0D3ECD";
  StringRef EvenData(reinterpret_cast<const char *>(EvenBytes.data()),
                     EvenBytes.size());
  EXPECT_EQ(EvenStr, to_hex(EvenData));
  EXPECT_EQ(EvenData, from_hex(EvenStr));
}

TEST(StringExtrasTest, to_float)
{
  float F;
  EXPECT_TRUE(to_float("4.7", F));
  EXPECT_FLOAT_EQ(4.7f, F);

  double D;
  EXPECT_TRUE(to_float("4.7", D));
  EXPECT_DOUBLE_EQ(4.7, D);

  long double LD;
  EXPECT_TRUE(to_float("4.7", LD));
  EXPECT_DOUBLE_EQ(4.7, LD);

  EXPECT_FALSE(to_float("foo", F));
  EXPECT_FALSE(to_float("7.4 foo", F));
  EXPECT_FLOAT_EQ(4.7f, F); // F should be unchanged
}

TEST(StringExtrasTest, printLowerCase)
{
   // unittest mark
//  std::string str;
//  RawStringOutStream OS(str);
//  printLowerCase("ABCdefg01234.,&!~`'}\"", OS);
//  EXPECT_EQ("abcdefg01234.,&!~`'}\"", OS.str());
}

}

