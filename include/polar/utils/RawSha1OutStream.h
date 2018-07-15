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

#ifndef POLAR_UTILS_RAW_SHA1_OUT_STREAM_H
#define POLAR_UTILS_RAW_SHA1_OUT_STREAM_H

#include "polar/basic/adt/ArrayRef.h"
#include "polar/utils/Sha1.h"
#include "polar/utils/RawOutStream.h"

namespace polar {
namespace utils {

/// A RawOutStream that hash the content using the getSha1 algorithm.
class RawSha1OutStream : public RawOutStream
{
   Sha1 m_state;

   /// See RawOutStream::writeImpl.
   void writeImpl(const char *ptr, size_t size) override
   {
      m_state.update(ArrayRef<uint8_t>((const uint8_t *)ptr, size));
   }

public:
   /// Return the current SHA1 hash for the content of the stream
   StringRef getSha1()
   {
      flush();
      return m_state.result();
   }

   /// Reset the internal state to start over from scratch.
   void resetHash()
   {
      m_state.init();
   }

   uint64_t getCurrentPos() const override
   {
      return 0;
   }
};

} // utils
} // polar

#endif // POLAR_UTILS_RAW_SHA1_OUT_STREAM_H
