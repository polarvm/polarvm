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

#ifndef QS_PARSER_LIB_ENGINE_H
#define QS_PARSER_LIB_ENGINE_H

#include <list>
#include <string>
#include <experimental/filesystem>

namespace qsparser {

class HelperProvider;

class Engine
{
public:
   Engine(const std::string &outputDir, const std::string &tplDir);
   void parser();
   void registerHelper(HelperProvider &provider);

protected:
   void loadTplFiles();
   void generateOutputFilename();
   void processOutputTags();
   void processScriptTags();
protected:
   std::string m_outputDir;
};

}

#endif // QS_PARSER_LIB_ENGINE_H
