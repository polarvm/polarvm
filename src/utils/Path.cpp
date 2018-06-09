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

#include "polar/utils/Path.h"
#include "polar/basic/adt/ArrayRef.h"
#include "polar/global/PolarConfig.h"
#include "polar/utils/Endian.h"
#include "polar/utils/ErrorCode.h"
#include "polar/utils/ErrorHandling.h"
#include "polar/utils/FileSystem.h"
#include "polar/utils/Process.h"
#include "polar/utils/Signals.h"
#include <cctype>
#include <cstring>

#if !defined(_MSC_VER) && !defined(__MINGW32__)
#include <unistd.h>
#else
#include <io.h>
#endif

namespace polar {
namespace fs {
namespace path {

using namespace polar::utils::endian;
using polar::basic::StringRef;
using polar::fs::path::is_separator;
using polar::fs::path::Style;
using polar::utils::Process;

enum FSEntity
{
   FS_Dir,
   FS_File,
   FS_Name
};

namespace {


inline Style real_style(Style style)
{
#ifdef _WIN32
   return (style == Style::posix) ? Style::posix : Style::windows;
#else
   return (style == Style::windows) ? Style::windows : Style::posix;
#endif
}

inline const char *separators(Style style)
{
   if (real_style(style) == Style::windows) {
      return "\\/";
   }
   return "/";
}

inline char preferred_separator(Style style) {
   if (real_style(style) == Style::windows) {
      return '\\';
   }
   return '/';
}

StringRef find_first_component(StringRef path, Style style)
{
   // Look for this first component in the following order.
   // * empty (in this case we return an empty string)
   // * either C: or {//,\\}net.
   // * {/,\}
   // * {file,directory}name

   if (path.empty()) {
      return path;
   }
   if (real_style(style) == Style::windows) {
      // C:
      if (path.getSize() >= 2 &&
          std::isalpha(static_cast<unsigned char>(path[0])) && path[1] == ':') {
         return path.substr(0, 2);
      }
   }

   // //net
   if ((path.getSize() > 2) && is_separator(path[0], style) &&
       path[0] == path[1] && !is_separator(path[2], style)) {
      // Find the next directory separator.
      size_t end = path.findFirstOf(separators(style), 2);
      return path.substr(0, end);
   }

   // {/,\}
   if (is_separator(path[0], style)) {
      return path.substr(0, 1);
   }

   // * {file,directory}name
   size_t end = path.findFirstOf(separators(style));
   return path.substr(0, end);
}

// Returns the first character of the filename in str. For paths ending in
// '/', it returns the position of the '/'.
size_t filename_pos(StringRef str, Style style)
{
   if (str.getSize() > 0 && is_separator(str[str.getSize() - 1], style)) {
      return str.getSize() - 1;
   }
   size_t pos = str.findLastOf(separators(style), str.getSize() - 1);

   if (real_style(style) == Style::windows) {
      if (pos == StringRef::npos) {
         pos = str.findLastOf(':', str.getSize() - 2);
      }
   }
   if (pos == StringRef::npos || (pos == 1 && is_separator(str[0], style))) {
      return 0;
   }
   return pos + 1;
}

// Returns the position of the root directory in str. If there is no root
// directory in str, it returns StringRef::npos.
size_t root_dir_start(StringRef str, Style style)
{
   // case "c:/"
   if (real_style(style) == Style::windows) {
      if (str.getSize() > 2 && str[1] == ':' && is_separator(str[2], style)) {
         return 2;
      }
   }

   // case "//net"
   if (str.getSize() > 3 && is_separator(str[0], style) && str[0] == str[1] &&
       !is_separator(str[2], style)) {
      return str.findFirstOf(separators(style), 2);
   }

   // case "/"
   if (str.getSize() > 0 && is_separator(str[0], style)) {
      return 0;
   }
   return StringRef::npos;
}

// Returns the position past the end of the "parent path" of path. The parent
// path will not end in '/', unless the parent is the root directory. If the
// path has no parent, 0 is returned.
size_t parent_path_end(StringRef path, Style style)
{
   size_t endPos = filename_pos(path, style);
   bool filenameWasSep =
         path.getSize() > 0 && is_separator(path[endPos], style);

   // Skip separators until we reach root dir (or the start of the string).
   size_t rootDirPos = root_dir_start(path, style);
   while (endPos > 0 &&
          (rootDirPos == StringRef::npos || endPos > rootDirPos) &&
          is_separator(path[endPos - 1], style)) {
      --endPos;
   }


   if (endPos == rootDirPos && !filenameWasSep) {
      // We've reached the root dir and the input path was *not* ending in a
      // sequence of slashes. Include the root dir in the parent path.
      return rootDirPos + 1;
   }

   // Otherwise, just include before the last slash.
   return endPos;
}

std::error_code create_unique_entity(const Twine &model, int &resultFD,
                                     SmallVectorImpl<char> &resultPath, bool makeAbsolute,
                                     unsigned mode, FSEntity type,
                                     polar::fs::OpenFlags flags = polar::fs::F_None) {
   SmallString<128> modelStorage;
   model.toVector(modelStorage);

   if (makeAbsolute) {
      // Make model absolute by prepending a temp directory if it's not already.
      if (!path::is_absolute(Twine(modelStorage))) {
         SmallString<128> tdir;
         path::system_temp_directory(true, tdir);
         path::append(tdir, Twine(modelStorage));
         modelStorage.swap(tdir);
      }
   }

   // From here on, DO NOT modify model. It may be needed if the randomly chosen
   // path already exists.
   resultPath = modelStorage;
   // Null terminate.
   resultPath.push_back(0);
   resultPath.pop_back();

retry_random_path:
   // Replace '%' with random chars.
   for (unsigned i = 0, e = modelStorage.getSize(); i != e; ++i) {
      if (modelStorage[i] == '%') {
         resultPath[i] = "0123456789abcdef"[Process::getRandomNumber() & 15];
      }
   }

   // Try to open + create the file.
   switch (type) {
   case FS_File: {
      if (std::error_code errorCode =
          fs::open_file_for_write(Twine(resultPath.begin()), resultFD,
                                  flags | fs::F_Excl, mode)) {
         if (errorCode == ErrorCode::file_exists)
            goto retry_random_path;
         return errorCode;
      }

      return std::error_code();
   }

   case FS_Name: {
      std::error_code errorCode =
            fs::access(resultPath.begin(), fs::AccessMode::Exist);
      if (errorCode == ErrorCode::no_such_file_or_directory) {
         return std::error_code();
      }
      if (errorCode) {
         return errorCode;
      }

      goto retry_random_path;
   }

   case FS_Dir: {
      if (std::error_code errorCode =
          fs::create_directory(resultPath.begin(), false)) {
         if (errorCode == ErrorCode::file_exists) {
            goto retry_random_path;
         }
         return errorCode;
      }
      return std::error_code();
   }
   }
   polar_unreachable("Invalid Type");
}

} // end unnamed namespace



} // path
} // fs
} // polar
