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

#include "polar/utils/EndianStream.h"
#include "polar/basic/adt/SmallString.h"
#include "polar/global/DataTypes.h"
#include "gtest/gtest.h"

using namespace polar::basic;
using namespace polar::utils;

namespace {

TEST(EndianStreamTest, testWriteInt32LE)
{
   SmallString<16> data;

   {
      RawSvectorOutStream outstream(data);
      endian::Writer<Endianness::Little> LE(outstream);
      LE.write(static_cast<int32_t>(-1362446643));
   }

   EXPECT_EQ(static_cast<uint8_t>(data[0]), 0xCD);
   EXPECT_EQ(static_cast<uint8_t>(data[1]), 0xB6);
   EXPECT_EQ(static_cast<uint8_t>(data[2]), 0xCA);
   EXPECT_EQ(static_cast<uint8_t>(data[3]), 0xAE);
}

TEST(EndianStreamTest, testWriteInt32BE)
{
   SmallVector<char, 16> data;

   {
      RawSvectorOutStream outstream(data);
      endian::Writer<Endianness::Big> BE(outstream);
      BE.write(static_cast<int32_t>(-1362446643));
   }

   EXPECT_EQ(static_cast<uint8_t>(data[0]), 0xAE);
   EXPECT_EQ(static_cast<uint8_t>(data[1]), 0xCA);
   EXPECT_EQ(static_cast<uint8_t>(data[2]), 0xB6);
   EXPECT_EQ(static_cast<uint8_t>(data[3]), 0xCD);
}


TEST(EndianStreamTest, testWriteFloatLE)
{
   SmallString<16> data;

   {
      RawSvectorOutStream outstream(data);
      endian::Writer<Endianness::Little> LE(outstream);
      LE.write(12345.0f);
   }

   EXPECT_EQ(static_cast<uint8_t>(data[0]), 0x00);
   EXPECT_EQ(static_cast<uint8_t>(data[1]), 0xE4);
   EXPECT_EQ(static_cast<uint8_t>(data[2]), 0x40);
   EXPECT_EQ(static_cast<uint8_t>(data[3]), 0x46);
}

TEST(EndianStreamTest, testWriteFloatBE)
{
   SmallVector<char, 16> data;

   {
      RawSvectorOutStream outstream(data);
      endian::Writer<Endianness::Big> BE(outstream);
      BE.write(12345.0f);
   }

   EXPECT_EQ(static_cast<uint8_t>(data[0]), 0x46);
   EXPECT_EQ(static_cast<uint8_t>(data[1]), 0x40);
   EXPECT_EQ(static_cast<uint8_t>(data[2]), 0xE4);
   EXPECT_EQ(static_cast<uint8_t>(data[3]), 0x00);
}

TEST(EndianStreamTest, testWriteInt64LE)
{
   SmallString<16> data;

   {
      RawSvectorOutStream outstream(data);
      endian::Writer<Endianness::Little> LE(outstream);
      LE.write(static_cast<int64_t>(-136244664332342323));
   }

   EXPECT_EQ(static_cast<uint8_t>(data[0]), 0xCD);
   EXPECT_EQ(static_cast<uint8_t>(data[1]), 0xAB);
   EXPECT_EQ(static_cast<uint8_t>(data[2]), 0xED);
   EXPECT_EQ(static_cast<uint8_t>(data[3]), 0x1B);
   EXPECT_EQ(static_cast<uint8_t>(data[4]), 0x33);
   EXPECT_EQ(static_cast<uint8_t>(data[5]), 0xF6);
   EXPECT_EQ(static_cast<uint8_t>(data[6]), 0x1B);
   EXPECT_EQ(static_cast<uint8_t>(data[7]), 0xFE);
}

TEST(EndianStreamTest, testWriteInt64BE)
{
   SmallVector<char, 16> data;

   {
      RawSvectorOutStream outstream(data);
      endian::Writer<Endianness::Big> BE(outstream);
      BE.write(static_cast<int64_t>(-136244664332342323));
   }

   EXPECT_EQ(static_cast<uint8_t>(data[0]), 0xFE);
   EXPECT_EQ(static_cast<uint8_t>(data[1]), 0x1B);
   EXPECT_EQ(static_cast<uint8_t>(data[2]), 0xF6);
   EXPECT_EQ(static_cast<uint8_t>(data[3]), 0x33);
   EXPECT_EQ(static_cast<uint8_t>(data[4]), 0x1B);
   EXPECT_EQ(static_cast<uint8_t>(data[5]), 0xED);
   EXPECT_EQ(static_cast<uint8_t>(data[6]), 0xAB);
   EXPECT_EQ(static_cast<uint8_t>(data[7]), 0xCD);
}

TEST(EndianStreamTest, testWriteDoubleLE)
{
   SmallString<16> data;

   {
      RawSvectorOutStream outstream(data);
      endian::Writer<Endianness::Little> LE(outstream);
      LE.write(-2349214918.58107);
   }

   EXPECT_EQ(static_cast<uint8_t>(data[0]), 0x20);
   EXPECT_EQ(static_cast<uint8_t>(data[1]), 0x98);
   EXPECT_EQ(static_cast<uint8_t>(data[2]), 0xD2);
   EXPECT_EQ(static_cast<uint8_t>(data[3]), 0x98);
   EXPECT_EQ(static_cast<uint8_t>(data[4]), 0xC5);
   EXPECT_EQ(static_cast<uint8_t>(data[5]), 0x80);
   EXPECT_EQ(static_cast<uint8_t>(data[6]), 0xE1);
   EXPECT_EQ(static_cast<uint8_t>(data[7]), 0xC1);
}

TEST(EndianStreamTest, testWriteDoubleBE)
{
   SmallVector<char, 16> data;

   {
      RawSvectorOutStream outstream(data);
      endian::Writer<Endianness::Big> BE(outstream);
      BE.write(-2349214918.58107);
   }

   EXPECT_EQ(static_cast<uint8_t>(data[0]), 0xC1);
   EXPECT_EQ(static_cast<uint8_t>(data[1]), 0xE1);
   EXPECT_EQ(static_cast<uint8_t>(data[2]), 0x80);
   EXPECT_EQ(static_cast<uint8_t>(data[3]), 0xC5);
   EXPECT_EQ(static_cast<uint8_t>(data[4]), 0x98);
   EXPECT_EQ(static_cast<uint8_t>(data[5]), 0xD2);
   EXPECT_EQ(static_cast<uint8_t>(data[6]), 0x98);
   EXPECT_EQ(static_cast<uint8_t>(data[7]), 0x20);
}

TEST(EndianStreamTest, testWriteArrayLE)
{
   SmallString<16> Data;

   {
      RawSvectorOutStream outstream(Data);
      endian::Writer<Endianness::Little> LE(outstream);
      LE.write<uint16_t>({0x1234, 0x5678});
   }

   EXPECT_EQ(static_cast<uint8_t>(Data[0]), 0x34);
   EXPECT_EQ(static_cast<uint8_t>(Data[1]), 0x12);
   EXPECT_EQ(static_cast<uint8_t>(Data[2]), 0x78);
   EXPECT_EQ(static_cast<uint8_t>(Data[3]), 0x56);
}

TEST(EndianStreamTest, testWriteVectorLE)
{
   SmallString<16> Data;

   {
      RawSvectorOutStream outstream(Data);
      endian::Writer<Endianness::Little> LE(outstream);
      std::vector<uint16_t> Vec{0x1234, 0x5678};
      LE.write<uint16_t>(Vec);
   }

   EXPECT_EQ(static_cast<uint8_t>(Data[0]), 0x34);
   EXPECT_EQ(static_cast<uint8_t>(Data[1]), 0x12);
   EXPECT_EQ(static_cast<uint8_t>(Data[2]), 0x78);
   EXPECT_EQ(static_cast<uint8_t>(Data[3]), 0x56);
}

TEST(EndianStreamTest, testWriteFloatArrayLE)
{
   SmallString<16> Data;

   {
      RawSvectorOutStream outstream(Data);
      endian::Writer<Endianness::Little> LE(outstream);
      LE.write<float>({12345.0f, 12346.0f});
   }

   EXPECT_EQ(static_cast<uint8_t>(Data[0]), 0x00);
   EXPECT_EQ(static_cast<uint8_t>(Data[1]), 0xE4);
   EXPECT_EQ(static_cast<uint8_t>(Data[2]), 0x40);
   EXPECT_EQ(static_cast<uint8_t>(Data[3]), 0x46);

   EXPECT_EQ(static_cast<uint8_t>(Data[4]), 0x00);
   EXPECT_EQ(static_cast<uint8_t>(Data[5]), 0xE8);
   EXPECT_EQ(static_cast<uint8_t>(Data[6]), 0x40);
   EXPECT_EQ(static_cast<uint8_t>(Data[7]), 0x46);
}

} // anonymous namespace

