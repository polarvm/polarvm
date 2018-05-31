// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
// 
// Created by softboy on 2018/05/30.

#ifndef POLAR_UTILS_RAW_OS_OUT_STREAM_H
#define POLAR_UTILS_RAW_OS_OUT_STREAM_H

#include "polar/utils/RawOutStream.h"
#include <iosfwd>

namespace polar {
namespace utils {

/// raw_os_ostream - A raw_ostream that writes to an std::ostream.  This is a
/// simple adaptor class.  It does not check for output errors; clients should
/// use the underlying stream to detect errors.
class RawOsOutStream : public RawOutStream
{
   std::ostream &m_outStream;
   
   /// write_impl - See raw_ostream::write_impl.
   void writeImpl(const char *ptr, size_t size) override;
   
   /// current_pos - Return the current position within the stream, not
   /// counting the bytes currently in the buffer.
   uint64_t getCurrentPos() const override;
   
public:
   RawOsOutStream(std::ostream &outStream)
      : m_outStream(outStream)
   {}
   
   ~RawOsOutStream() override;
};

} // utils
} // polar

#endif // POLAR_UTILS_RAW_OS_OUT_STREAM_H
