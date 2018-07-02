// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/07/02.

#include "polar/utils/BinaryStreamError.h"
#include "polar/utils/ErrorHandling.h"

namespace polar {
namespace utils {

char BinaryStreamError::m_id = 0;

BinaryStreamError::BinaryStreamError(StreamErrorCode code)
   : BinaryStreamError(code, "")
{}

BinaryStreamError::BinaryStreamError(StringRef context)
   : BinaryStreamError(StreamErrorCode::unspecified, context)
{}

BinaryStreamError::BinaryStreamError(StreamErrorCode code, StringRef context)
   : m_code(code)
{
   m_errorMsg = "Stream Error: ";
   switch (code) {
   case StreamErrorCode::unspecified:
      m_errorMsg += "An unspecified error has occurred.";
      break;
   case StreamErrorCode::stream_too_short:
      m_errorMsg += "The stream is too short to perform the requested operation.";
      break;
   case StreamErrorCode::invalid_array_size:
      m_errorMsg += "The buffer size is not a multiple of the array element size.";
      break;
   case StreamErrorCode::invalid_offset:
      m_errorMsg += "The specified offset is invalid for the current stream.";
      break;
   case StreamErrorCode::filesystem_error:
      m_errorMsg += "An I/O error occurred on the file system.";
      break;
   }

   if (!context.empty()) {
      m_errorMsg += "  ";
      m_errorMsg += context;
   }
}

void BinaryStreamError::log(RawOutStream &outstream) const
{
   outstream << m_errorMsg << "\n";
}

StringRef BinaryStreamError::getErrorMessage() const
{
   return m_errorMsg;
}

std::error_code BinaryStreamError::convertToErrorCode() const
{
   return inconvertible_error_code();
}

} // utils
} // polar
