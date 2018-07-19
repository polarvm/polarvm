// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/07/17.

#include "polar/utils/CommandLine.h"
#include "lib/Engine.h"
#include <iostream>

using namespace polar;
using qsparser::Engine;

int main(int argc, char *argv[])
{

   cmd::Opt<std::string> outputDir("d", cmd::Desc("output intermediate files"), cmd::ValueDesc("output dir"), cmd::Required);
   cmd::List<std::string> fileOpts(cmd::Positional, cmd::Desc("<input files>"), cmd::OneOrMore);
   cmd::parse_command_line_options(argc, argv, "welcome use QsParser");
   std::list<std::string> files;
   for (unsigned i = 0; i != fileOpts.getSize(); ++i) {
      files.push_back(fileOpts[i]);
   }
   Engine engine(outputDir.getValue());
   if(!engine.parser(files)) {
      std::cerr << engine.getErrorMsg() << std::endl;
      return -1;
   }
   return 0;
}
