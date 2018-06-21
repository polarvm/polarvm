// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/06/21.

//===----------------------------------------------------------------------===//
//
// Utility for creating a in-memory buffer that will be written to a file.
//
//===----------------------------------------------------------------------===//

#ifndef POLAR_UTILS_FILE_OUTPUT_BUFFER_H
#define POLAR_UTILS_FILE_OUTPUT_BUFFER_H

#include "polar/basic/adt/SmallString.h"
#include "polar/basic/adt/StringRef.h"
#include "polar/global/DataTypes.h"
#include "polar/utils/ErrorType.h"
#include "polar/utils/FileSystem.h"

namespace polar {
namespace utils {

/// FileOutputBuffer - This interface provides simple way to create an in-memory
/// buffer which will be written to a file. During the lifetime of these
/// objects, the content or existence of the specified file is undefined. That
/// is, creating an OutputBuffer for a file may immediately remove the file.
/// If the FileOutputBuffer is committed, the target file's content will become
/// the buffer content at the time of the commit.  If the FileOutputBuffer is
/// not committed, the file will be deleted in the FileOutputBuffer destructor.
class FileOutputBuffer
{
public:
   enum  {
      F_executable = 1  /// set the 'x' bit on the resulting file
   };

   /// Factory method to create an OutputBuffer object which manages a read/write
   /// buffer of the specified size. When committed, the buffer will be written
   /// to the file at the specified path.
   static Expected<std::unique_ptr<FileOutputBuffer>>
   create(StringRef filePath, size_t size, unsigned flags = 0);

   /// Returns a pointer to the start of the buffer.
   virtual uint8_t *getBufferStart() const = 0;

   /// Returns a pointer to the end of the buffer.
   virtual uint8_t *getBufferEnd() const = 0;

   /// Returns size of the buffer.
   virtual size_t getBufferSize() const = 0;

   /// Returns path where file will show up if buffer is committed.
   StringRef getPath() const
   {
      return m_finalPath;
   }

   /// Flushes the content of the buffer to its file and deallocates the
   /// buffer.  If commit() is not called before this object's destructor
   /// is called, the file is deleted in the destructor. The optional parameter
   /// is used if it turns out you want the file size to be smaller than
   /// initially requested.
   virtual Error commit() = 0;

   /// If this object was previously committed, the destructor just deletes
   /// this object.  If this object was not committed, the destructor
   /// deallocates the buffer and the target file is never written.
   virtual ~FileOutputBuffer()
   {}

protected:
   FileOutputBuffer(StringRef path)
      : m_finalPath(path)
   {}

   std::string m_finalPath;
};

} // utils
} // polar

#endif // POLAR_UTILS_FILE_OUTPUT_BUFFER_H
