// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/07/15.

#ifndef POLAR_UTILS_TAR_WRITER_H
#define POLAR_UTILS_TAR_WRITER_H

#include "polar/basic/adt/StringRef.h"
#include "polar/basic/adt/StringSet.h"
#include "polar/utils/ErrorType.h"
#include "polar/utils/RawOutStream.h"

namespace polar {
namespace utils {

using polar::basic::StringSet;

class TarWriter {
public:
   static Expected<std::unique_ptr<TarWriter>> create(StringRef outputPath,
                                                      StringRef baseDir);

   void append(StringRef path, StringRef data);

private:
   TarWriter(int fd, StringRef baseDir);
   RawFdOutStream m_outstream;
   std::string m_baseDir;
   StringSet<> m_files;
};

} // utils
} // polar

#endif // POLAR_UTILS_TAR_WRITER_H
