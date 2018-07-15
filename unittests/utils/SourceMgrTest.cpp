// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/07/15.

#include "polar/utils/SourceMgr.h"
#include "polar/utils/MemoryBuffer.h"
#include "polar/utils/RawOutStream.h"
#include "gtest/gtest.h"

using namespace polar;
using namespace polar::utils;
using namespace polar::basic;

namespace {

class SourceMgrTest : public testing::Test
{
public:
   SourceMgr SM;
   unsigned mainBufferID;
   std::string output;

   void setMainBuffer(StringRef text, StringRef bufferName)
   {
      std::unique_ptr<MemoryBuffer> MainBuffer =
            MemoryBuffer::getMemBuffer(text, bufferName);
      mainBufferID = SM.addNewSourceBuffer(std::move(MainBuffer), polar::utils::SMLocation());
   }

   SMLocation getLoc(unsigned offset)
   {
      return SMLocation::getFromPointer(
               SM.getMemoryBuffer(mainBufferID)->getBufferStart() + offset);
   }

   SMRange getRange(unsigned offset, unsigned Length)
   {
      return SMRange(getLoc(offset), getLoc(offset + Length));
   }

   void printMessage(SMLocation Loc, SourceMgr::DiagKind Kind,
                     const Twine &Msg, ArrayRef<SMRange> Ranges,
                     ArrayRef<SMFixIt> FixIts)
   {
      RawStringOutStream OS(output);
      SM.printMessage(OS, Loc, Kind, Msg, Ranges, FixIts);
   }
};

TEST_F(SourceMgrTest, testBasicError)
{
   setMainBuffer("aaa bbb\nccc ddd\n", "file.in");
   printMessage(getLoc(4), SourceMgr::DK_Error, "message", std::nullopt, std::nullopt);

   EXPECT_EQ("file.in:1:5: error: message\n"
             "aaa bbb\n"
             "    ^\n",
             output);
}

TEST_F(SourceMgrTest, testBasicWarning)
{
   setMainBuffer("aaa bbb\nccc ddd\n", "file.in");
   printMessage(getLoc(4), SourceMgr::DK_Warning, "message", std::nullopt, std::nullopt);

   EXPECT_EQ("file.in:1:5: warning: message\n"
             "aaa bbb\n"
             "    ^\n",
             output);
}

TEST_F(SourceMgrTest, testBasicRemark)
{
   setMainBuffer("aaa bbb\nccc ddd\n", "file.in");
   printMessage(getLoc(4), SourceMgr::DK_Remark, "message", std::nullopt, std::nullopt);

   EXPECT_EQ("file.in:1:5: remark: message\n"
             "aaa bbb\n"
             "    ^\n",
             output);
}

TEST_F(SourceMgrTest, testBasicNote)
{
   setMainBuffer("aaa bbb\nccc ddd\n", "file.in");
   printMessage(getLoc(4), SourceMgr::DK_Note, "message", std::nullopt, std::nullopt);

   EXPECT_EQ("file.in:1:5: note: message\n"
             "aaa bbb\n"
             "    ^\n",
             output);
}

TEST_F(SourceMgrTest, testLocationAtEndOfLine)
{
   setMainBuffer("aaa bbb\nccc ddd\n", "file.in");
   printMessage(getLoc(6), SourceMgr::DK_Error, "message", std::nullopt, std::nullopt);

   EXPECT_EQ("file.in:1:7: error: message\n"
             "aaa bbb\n"
             "      ^\n",
             output);
}

TEST_F(SourceMgrTest, testLocationAtNewline)
{
   setMainBuffer("aaa bbb\nccc ddd\n", "file.in");
   printMessage(getLoc(7), SourceMgr::DK_Error, "message", std::nullopt, std::nullopt);

   EXPECT_EQ("file.in:1:8: error: message\n"
             "aaa bbb\n"
             "       ^\n",
             output);
}

TEST_F(SourceMgrTest, testBasicRange)
{
   setMainBuffer("aaa bbb\nccc ddd\n", "file.in");
   printMessage(getLoc(4), SourceMgr::DK_Error, "message", getRange(4, 3), std::nullopt);

   EXPECT_EQ("file.in:1:5: error: message\n"
             "aaa bbb\n"
             "    ^~~\n",
             output);
}

TEST_F(SourceMgrTest, testRangeWithTab)
{
   setMainBuffer("aaa\tbbb\nccc ddd\n", "file.in");
   printMessage(getLoc(4), SourceMgr::DK_Error, "message", getRange(3, 3), std::nullopt);

   EXPECT_EQ("file.in:1:5: error: message\n"
             "aaa     bbb\n"
             "   ~~~~~^~\n",
             output);
}

TEST_F(SourceMgrTest, testMultiLineRange)
{
   setMainBuffer("aaa bbb\nccc ddd\n", "file.in");
   printMessage(getLoc(4), SourceMgr::DK_Error, "message", getRange(4, 7), std::nullopt);

   EXPECT_EQ("file.in:1:5: error: message\n"
             "aaa bbb\n"
             "    ^~~\n",
             output);
}

TEST_F(SourceMgrTest, testMultipleRanges)
{
   setMainBuffer("aaa bbb\nccc ddd\n", "file.in");
   SMRange Ranges[] = { getRange(0, 3), getRange(4, 3) };
   printMessage(getLoc(4), SourceMgr::DK_Error, "message", Ranges, std::nullopt);

   EXPECT_EQ("file.in:1:5: error: message\n"
             "aaa bbb\n"
             "~~~ ^~~\n",
             output);
}

TEST_F(SourceMgrTest, testOverlappingRanges)
{
   setMainBuffer("aaa bbb\nccc ddd\n", "file.in");
   SMRange Ranges[] = { getRange(0, 3), getRange(2, 4) };
   printMessage(getLoc(4), SourceMgr::DK_Error, "message", Ranges, std::nullopt);

   EXPECT_EQ("file.in:1:5: error: message\n"
             "aaa bbb\n"
             "~~~~^~\n",
             output);
}

TEST_F(SourceMgrTest, testBasicFixit) {
   setMainBuffer("aaa bbb\nccc ddd\n", "file.in");
   printMessage(getLoc(4), SourceMgr::DK_Error, "message", std::nullopt,
                make_array_ref(SMFixIt(getRange(4, 3), "zzz")));

   EXPECT_EQ("file.in:1:5: error: message\n"
             "aaa bbb\n"
             "    ^~~\n"
             "    zzz\n",
             output);
}

TEST_F(SourceMgrTest, testFixitForTab)
{
   setMainBuffer("aaa\tbbb\nccc ddd\n", "file.in");
   printMessage(getLoc(3), SourceMgr::DK_Error, "message", std::nullopt,
                make_array_ref(SMFixIt(getRange(3, 1), "zzz")));

   EXPECT_EQ("file.in:1:4: error: message\n"
             "aaa     bbb\n"
             "   ^^^^^\n"
             "   zzz\n",
             output);
}

} // anonymous namespace
