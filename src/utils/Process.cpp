// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/06/08.

//===----------------------------------------------------------------------===//
//
//  This file implements the operating system Process concept.
//
//===----------------------------------------------------------------------===//

#include "polar/utils/Process.h"
#include "polar/basic/adt/StlExtras.h"
#include "polar/basic/adt/StringExtras.h"
#include "polar/global/PolarConfig.h"
#include "polar/utils/FileSystem.h"
#include "polar/utils/Path.h"
#include "polar/utils/Program.h"

#include <optional>

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

namespace polar {
namespace sys {

namespace path = fs::path;
using polar::basic::SmallVector;
using polar::basic::SmallString;
using polar::basic::Twine;

std::optional<std::string> Process::findInEnvPath(StringRef envName,
                                                  StringRef fileName)
{
   return findInEnvPath(envName, fileName, {});
}

std::optional<std::string> Process::findInEnvPath(StringRef envName,
                                                  StringRef fileName,
                                                  ArrayRef<std::string> ignoreList)
{
   assert(!path::is_absolute(fileName));
   std::optional<std::string> foundPath;
   std::optional<std::string> optPath = Process::getEnv(envName);
   if (!optPath.has_value()) {
      return foundPath;
   }
   const char envPathSeparatorStr[] = {g_envPathSeparator, '\0'};
   SmallVector<StringRef, 8> dirs;
   polar::basic::split_string(optPath.value(), dirs, envPathSeparatorStr);

   for (StringRef dir : dirs) {
      if (dir.empty()) {
         continue;
      }
      if (polar::basic::any_of(ignoreList, [&](StringRef str) { return fs::equivalent(str, dir); })) {
         continue;
      }

      SmallString<128> filePath(dir);
      path::append(filePath, fileName);
      if (fs::exists(Twine(filePath))) {
         foundPath = filePath.getStr();
         break;
      }
   }
   return foundPath;
}


#define COLOR(FGBG, CODE, BOLD) "\033[0;" BOLD FGBG CODE "m"

#define ALLCOLORS(FGBG,BOLD) {\
   COLOR(FGBG, "0", BOLD),\
   COLOR(FGBG, "1", BOLD),\
   COLOR(FGBG, "2", BOLD),\
   COLOR(FGBG, "3", BOLD),\
   COLOR(FGBG, "4", BOLD),\
   COLOR(FGBG, "5", BOLD),\
   COLOR(FGBG, "6", BOLD),\
   COLOR(FGBG, "7", BOLD)\
}

extern const char sg_colorcodes[2][2][8][10] =
{
   { ALLCOLORS("3",""), ALLCOLORS("3","1;") },
   { ALLCOLORS("4",""), ALLCOLORS("4","1;") }
};

// This is set to true when Process::PreventCoreFiles() is called.
bool sg_coreFilesPrevented = false;

bool Process::areCoreFilesPrevented()
{
   return sg_coreFilesPrevented;
}

} // sys
} // polar
