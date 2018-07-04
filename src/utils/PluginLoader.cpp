// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/07/04.

#define DONT_GET_PLUGIN_LOADER_OPTION
#include "polar/utils/PluginLoader.h"
#include "polar/utils/DynamicLibrary.h"
#include "polar/global/ManagedStatic.h"
#include "polar/utils/RawOutStream.h"
#include <vector>
#include <mutex>

namespace polar {
namespace utils {

static ManagedStatic<std::vector<std::string> > sg_plugins;
static ManagedStatic<std::mutex> sg_pluginsLock;

void PluginLoader::operator=(const std::string &filename) {
   std::lock_guard locker(*sg_pluginsLock);
   std::string error;
   if (polar::sys::DynamicLibrary::loadLibraryPermanently(filename.c_str(), &error)) {
      error_stream() << "Error opening '" << filename << "': " << error
                     << "\n  -load request ignored.\n";
   } else {
      sg_plugins->push_back(filename);
   }
}

unsigned PluginLoader::getNumPlugins()
{
   std::lock_guard locker(*sg_pluginsLock);
   return sg_plugins.isConstructed() ? sg_plugins->size() : 0;
}

std::string &PluginLoader::getPlugin(unsigned num)
{
   std::lock_guard locker(*sg_pluginsLock);
   assert(sg_plugins.isConstructed() && num < sg_plugins->size() &&
          "Asking for an out of bounds plugin");
   return (*sg_plugins)[num];
}

} // utils
} // polar
