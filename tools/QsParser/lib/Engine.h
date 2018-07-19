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
#include "polar/basic/adt/StringMap.h"

namespace qsparser {

using polar::basic::StringMap;

class Engine
{
public:
   Engine(std::string outputDir);
   bool parser(const std::list<std::string> &files);
   bool getStatus()
   {
      return m_status;
   }

   std::string getErrorMsg()
   {
      return m_errorMgs;
   }
protected:
   void loadTplFiles();
   void getIncludeList(const std::list<std::string> &files);
   void generateOutputFilename();
   void processOutputTags();
   void processScriptTags();
   void prepareEnv(const std::list<std::string> &files);
   void generate(std::string &generatedCode);
   void parseFiles(std::string &generatedCode);
protected:
   std::list<std::string> m_headers;
   std::string m_outputDir;
   std::string m_tplDir;
   std::string m_errorMgs;
   StringMap<std::pair<std::string, std::string>> m_sourceFiles;
   std::string m_appTpl;
   std::string m_classTpl;
   std::string m_methodTpl;
   bool m_status;
};

}

#endif // QS_PARSER_LIB_ENGINE_H
