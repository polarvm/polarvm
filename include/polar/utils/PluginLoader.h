// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/07/01.

#ifndef POLAR_UTILS_PLIGIN_LOADER_H
#define POLAR_UTILS_PLIGIN_LOADER_H

#include "polar/utils/CommandLine.h"

namespace polar {
namespace utils {

struct PluginLoader
{
   void operator=(const std::string &filename);
   static unsigned getNumPlugins();
   static std::string& getPlugin(unsigned num);
};

#ifndef DONT_GET_PLUGIN_LOADER_OPTION
// This causes operator= above to be invoked for every -load option.
static cmd::Opt<PluginLoader, false, cmd::Parser<std::string>>
sg_loadOpt("load", cmd::ZeroOrMore, cmd::ValueDesc("pluginfilename"),
           cmd::Desc("Load the specified plugin"));
#endif

} // utils
} // polar

#endif // POLAR_UTILS_PLIGIN_LOADER_H
