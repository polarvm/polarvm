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

#include "Engine.h"
#include "polar/basic/adt/Twine.h"
#include "polar/basic/adt/StringRef.h"
#include "polar/basic/adt/SmallVector.h"
#include "polar/utils/FileSystem.h"
#include "polar/utils/FormatVariadic.h"
#include "polar/utils/Md5.h"
#include <iostream>
#include <fstream>
#include <streambuf>
#include <regex>

namespace qsparser {

using namespace polar;
using polar::basic::StringRef;
using polar::basic::SmallVector;

#define HEADER_MARK "__INCLUDES__"
#define GENERATOR_CLS_MARK "__GENERATOR_CLS__"
#define METHOD_PREPARE_MARK "__METHOD_PREPARE__"
#define METHOD_NAME_MARK "__METHOD_NAME__"
#define METHOD_GENERATE_CODE_MARK "__METHOD_GENERATE_CODE__"
#define CLS_GENERATOR_METHOD_LIST_MARK "__GENERATOR_METHOD_LIST__"
#define CLS_METHOD_LIST_MARK "__METHOD_LIST__"
#define BASE_DIR_MARK "__BASE_DIR__"

namespace {

void do_load_file(const std::string &filename, std::string &content)
{
   if (!fs::exists(filename)) {
      throw std::runtime_error(utils::formatv("file: {0} not exist", filename));
   }
   std::ifstream istrm(filename);
   if (!istrm.is_open()) {
      throw std::runtime_error(utils::formatv("open file: {0} failed", filename));
   }
   istrm.seekg(0, std::ios::end);
   content.reserve(istrm.tellg());
   istrm.seekg(0, std::ios::beg);
   content.assign(std::istreambuf_iterator<char>(istrm),
                  std::istreambuf_iterator<char>());
}

} // anonymous namespace

Engine::Engine(std::string outputDir)
   : m_outputDir(std::move(outputDir)),
     m_tplDir(QSPARSER_TPL_DIR),
     m_status(true)
{}

bool Engine::parser(const std::list<std::string> &files)
{
   try {
      prepareEnv(files);
      getIncludeList(files);
      std::string generatorCode;
      parseFiles(generatorCode);
      generate(generatorCode);
      return true;
   } catch (std::exception &e) {
      m_status = false;
      m_errorMgs = e.what();
      return false;
   }
}

void Engine::parseFiles(std::string &generatedCode)
{
   std::regex regex(R"(<%!([^\x00]+?)%>)", std::regex::ECMAScript);
   std::string methodBodies;
   std::string methodNames;
   for (StringMap<std::string>::iterator iter = m_sourceFiles.begin();
        iter != m_sourceFiles.end(); ++iter) {
      // 获取环境准备代码
      std::string methodName = "generate_" + iter->first().getStr();
      methodNames += methodName + "();\n";
      std::smatch m;
      std::string methodCode = m_methodTpl;
      methodCode.replace(methodCode.find(METHOD_NAME_MARK), strlen(METHOD_NAME_MARK), methodName);
      std::string &content = iter->m_second;
      if (std::regex_search(content, m, regex)) {
         StringRef result(m[1].str().c_str());
         result = result.trim();
         methodCode.replace(methodCode.find(METHOD_PREPARE_MARK), strlen(METHOD_PREPARE_MARK), result);
         content = std::regex_replace(content, regex, "");
      }
      std::string methodBodyCode;
      methodBodyCode.reserve(content.size());
      methodBodyCode = "std::string code = R\"(";
      int codeLength = content.size();
      int i = 0;
      while (i < codeLength) {
         int nextCharPos = std::min(i + 1, codeLength - 1);
         if (content[i] == '<') {
            if (content[nextCharPos] == '%') {
               size_t eqMarkPos = std::min(nextCharPos + 1, codeLength - 1);
               if (content[eqMarkPos] == '=') {
                  // 输出数据代码
                  i += 3;
                  methodBodyCode += "utils::formatv(\"{0}\", ";
                  while (content[i] != '%' && content[nextCharPos] != '>') {
                     methodBodyCode += content[i];
                     ++i;
                  }
                  methodBodyCode += ")";
               } else {
                  // 普通嵌入代码
                  i += 2;
                  methodBodyCode += ")\";\n";
                  while (content[i] != '%' && content[nextCharPos] != '>') {
                     methodBodyCode += content[i];
                     ++i;
                  }
                  methodBodyCode += "\ncode += R\"(";
               }
            } else {
               // 普通文本
               methodBodyCode += content[i];
               ++i;
            }
         } else if (content[i] == '%' && content[nextCharPos] == '>') {
            // 关闭标签
            i += 2;
         } else {
            // 普通文本
            methodBodyCode += content[i];
            ++i;
         }
      }
      methodBodyCode += ")\";\n";
      methodCode.replace(methodCode.find(METHOD_GENERATE_CODE_MARK), strlen(METHOD_GENERATE_CODE_MARK), methodBodyCode);
      methodBodies += methodCode;
   }
   generatedCode = m_classTpl;
   generatedCode.replace(generatedCode.find(CLS_GENERATOR_METHOD_LIST_MARK), strlen(CLS_GENERATOR_METHOD_LIST_MARK), methodNames);
   generatedCode.replace(generatedCode.find(CLS_METHOD_LIST_MARK), strlen(CLS_METHOD_LIST_MARK), methodBodies);
}

void Engine::generate(std::string &generatedCode)
{
   std::string generatorFilename = m_outputDir + "/qsgenerator.cpp";
   std::string main = m_appTpl;
   m_headers.sort();
   auto last = std::unique(m_headers.begin(), m_headers.end());
   m_headers.erase(last, m_headers.end());
   std::string headerText;
   for (const std::string &header : m_headers) {
      headerText += header + "\n";
   }
   main.replace(main.find(HEADER_MARK), strlen(HEADER_MARK), headerText);
   main.replace(main.find(GENERATOR_CLS_MARK), strlen(GENERATOR_CLS_MARK), generatedCode);
   main.replace(main.find(BASE_DIR_MARK), strlen(BASE_DIR_MARK), m_outputDir);
   std::ofstream ostrm(generatorFilename, std::ios_base::trunc);
   if (!ostrm.is_open()) {
      throw std::runtime_error(utils::formatv("open file: {0} failed", generatorFilename));
   }
   ostrm << main;
   if (!ostrm) {
      throw std::runtime_error(utils::formatv("write generate result error into : {0}", generatorFilename));
   }
   ostrm.close();
}

void Engine::getIncludeList(const std::list<std::string> &files)
{
   std::regex regex(R"(<%header([^\x00]+?)%>)", std::regex::ECMAScript);
   utils::Md5 hash;
   for (const std::string &file : files) {
      std::string source;
      do_load_file(file, source);
      std::smatch m;
      if (std::regex_search(source, m, regex)) {
         StringRef result(m[1].str().c_str());
         result = result.trim();
         SmallVector<StringRef, 10> curHeaders;
         result.split(curHeaders, "\n", -1, false);
         for (StringRef header : curHeaders) {
            m_headers.push_back(header);
         }
         source = std::regex_replace(source, regex, "");
      }
      hash.update(file);
      utils::Md5::Md5Result res;
      hash.final(res);
      m_sourceFiles[res.getDigest()] = std::move(source);
   }
}

void Engine::prepareEnv(const std::list<std::string> &files)
{
   if (!fs::exists(m_outputDir)) {
      fs::create_directory(m_outputDir);
   }
   if (!fs::exists(m_tplDir)) {
      throw std::runtime_error("qsparser: tpldir is not exist");
   }
   for (const std::string &file : files) {
      if (!fs::exists(file)) {
         throw std::runtime_error(utils::formatv("qs file: {0} not exist", file));
      }
   }
   loadTplFiles();
}

void Engine::loadTplFiles()
{
   do_load_file(m_tplDir + "/App.tpl", m_appTpl);
   do_load_file(m_tplDir + "/Class.tpl", m_classTpl);
   do_load_file(m_tplDir + "/Method.tpl", m_methodTpl);
}

} // qsparser
