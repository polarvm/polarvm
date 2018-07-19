// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/07/18.

#include "gtest/gtest.h"
#include "QsParser/lib/Engine.h"
#include "polar/basic/adt/SmallString.h"
#include "polar/utils/FileSystem.h"
#include <iostream>
#include "polar/basic/adt/Twine.h"

using qsparser::Engine;
using polar::basic::SmallString;

namespace {

TEST(QsParserTest, testParse)
{
   if (!polar::fs::exists(QSPARSER_OUTPUT_DIR)) {
      polar::fs::create_directory(QSPARSER_OUTPUT_DIR);
   }
   Engine engine(QSPARSER_OUTPUT_DIR);
   bool status = engine.parser({QSPARSER_DATA_DIR+std::string("/TypeDefs.qs")});
   if (!status) {
      std::cout << engine.getErrorMsg() << std::endl;
   }
}

} // anonymous namespace
