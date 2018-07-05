// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/07/05.

//===----------------------------------------------------------------------===//
//
// This implements the ToolOutputFile class.
//
//===----------------------------------------------------------------------===//

#include "polar/utils/ToolOutputFile.h"
#include "polar/utils/FileSystem.h"
#include "polar/utils/Signals.h"

namespace polar {
namespace utils {

ToolOutputFile::CleanupInstaller::CleanupInstaller(StringRef filename)
   : m_filename(filename), m_keep(false) {
   // Arrange for the file to be deleted if the process is killed.
   if (m_filename != "-") {
      sys::remove_file_on_signal(filename);
   }
}

ToolOutputFile::CleanupInstaller::~CleanupInstaller()
{
   // Delete the file if the client hasn't told us not to.
   if (!m_keep && m_filename != "-") {
      polar::fs::remove(m_filename);
   }
   // Ok, the file is successfully written and closed, or deleted. There's no
   // further need to clean it up on signals.
   if (m_filename != "-") {
      sys::dont_remove_file_on_signal(m_filename);
   }
}

ToolOutputFile::ToolOutputFile(StringRef filename, std::error_code &errorCode,
                               polar::fs::OpenFlags flags)
   : m_installer(filename), m_outstream(filename, errorCode, flags)
{
   // If open fails, no cleanup is needed.
   if (errorCode) {
      m_installer.m_keep = true;
   }
}

ToolOutputFile::ToolOutputFile(StringRef filename, int fd)
   : m_installer(filename), m_outstream(fd, true)
{}

} // utils
} // polar
