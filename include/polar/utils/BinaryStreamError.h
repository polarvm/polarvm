// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/06/28.

#ifndef POLAR_UTILS_BINARY_STREAM_ERROR_H
#define POLAR_UTILS_BINARY_STREAM_ERROR_H

#include "polar/basic/adt/StringRef.h"
#include "polar/utils/ErrorType.h"
#include <string>

namespace polar {
namespace utils {

enum class StreamErrorCode
{
   unspecified,
   stream_too_short,
   invalid_array_size,
   invalid_offset,
   filesystem_error
};

/// Base class for errors originating when parsing raw PDB files
class BinaryStreamError : public ErrorInfo<BinaryStreamError>
{
public:
   static char sm_id;
   explicit BinaryStreamError(StreamErrorCode errorCode);
   explicit BinaryStreamError(StringRef context);
   BinaryStreamError(StreamErrorCode errorCode, StringRef context);

   void log(RawOutStream &outstream) const override;
   std::error_code convertToErrorCode() const override;
   StringRef getErrorMessage() const;
   StreamErrorCode getErrorCode() const
   {
      return m_code;
   }

private:
   std::string m_errorMsg;
   StreamErrorCode m_code;
};

} // utils
} // polar

#endif // POLAR_UTILS_BINARY_STREAM_ERROR_H
