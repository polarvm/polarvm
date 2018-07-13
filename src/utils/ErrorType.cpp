// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/06/06.

#include "polar/utils/ErrorType.h"
#include "polar/basic/adt/Twine.h"
#include "polar/utils/ErrorHandling.h"
#include "polar/global/ManagedStatic.h"
#include <system_error>

namespace {

enum class ErrorErrorCode : int
{
   MultipleErrors = 1,
   InconvertibleError
};

// FIXME: This class is only here to support the transition to llvm::Error. It
// will be removed once this transition is complete. Clients should prefer to
// deal with the Error value directly, rather than converting to error_code.
class ErrorErrorCategory : public std::error_category
{
public:
   const char *name() const noexcept override
   {
      return "Error";
   }

   std::string message(int condition) const override
   {
      switch (static_cast<ErrorErrorCode>(condition)) {
      case ErrorErrorCode::MultipleErrors:
         return "Multiple errors";
      case ErrorErrorCode::InconvertibleError:
         return "Inconvertible error value. An error has occurred that could "
                "not be converted to a known std::error_code. Please file a "
                "bug.";
      }
      polar_unreachable("Unhandled error code");
   }
};

} // anonymous namespace

namespace polar {

static ManagedStatic<ErrorErrorCategory> sg_errorErrorCategory;

namespace utils {

void ErrorInfoBase::anchor()
{}
char ErrorInfoBase::sm_id = 0;
char ErrorList::sm_id = 0;
char ECError::sm_id = 0;
char StringError::sm_id = 0;

void log_all_unhandled_errors(Error error, RawOutStream &outStream, Twine errorBanner) {
   if (!error) {
      return;
   }
   outStream << errorBanner;
   handle_all_errors(std::move(error), [&](const ErrorInfoBase &errorInfo) {
      errorInfo.log(outStream);
      outStream << "\n";
   });
}

std::error_code ErrorList::convertToErrorCode() const
{
   return std::error_code(static_cast<int>(ErrorErrorCode::MultipleErrors),
                          *sg_errorErrorCategory);
}

std::error_code inconvertible_error_code()
{
   return std::error_code(static_cast<int>(ErrorErrorCode::InconvertibleError),
                          *sg_errorErrorCategory);
}

Error error_code_to_error(std::error_code errorCode)
{
   if (!errorCode) {
      return Error::getSuccess();
   }
   return Error(std::make_unique<ECError>(ECError(errorCode)));
}

std::error_code error_to_error_code(Error error) {
   std::error_code errorCode;
   handle_all_errors(std::move(error), [&](const ErrorInfoBase &errorInfo) {
      errorCode = errorInfo.convertToErrorCode();
   });
   if (errorCode == inconvertible_error_code()) {
      report_fatal_error(errorCode.message());
   }
   return errorCode;
}

#if POLAR_ENABLE_ABI_BREAKING_CHECKS
void Error::fatalUncheckedError() const
{
   debug_stream() << "Program aborted due to an unhandled Error:\n";
   if (getPtr()) {
      getPtr()->log(debug_stream());
   } else {
      debug_stream() << "Error value was Success. (Note: Success values must still be "
                        "checked prior to being destroyed).\n";
   }
   abort();
}
#endif

StringError::StringError(const Twine &twine, std::error_code errorCode)
   : m_msg(twine.getStr()), m_errorCode(errorCode)
{}

void StringError::log(RawOutStream &outStream) const
{
   outStream << m_msg;
}

std::error_code StringError::convertToErrorCode() const
{
   return m_errorCode;
}

void report_fatal_error(Error error, bool genCrashDiag)
{
   assert(error && "report_fatal_error called with success value");
   std::string errMsg;
   {
      RawStringOutStream errorStream(errMsg);
      log_all_unhandled_errors(std::move(error), errorStream, "");
   }
   report_fatal_error(errMsg);
}

} // utils

#ifndef _MSC_VER
// One of these two variables will be referenced by a symbol defined in
// llvm-config.h. We provide a link-time (or load time for DSO) failure when
// there is a mismatch in the build configuration of the API client and LLVM.
#if POLAR_ENABLE_ABI_BREAKING_CHECKS
int g_enableABIBreakingChecks;
#else
int g_disableABIBreakingChecks;
#endif

#endif

} // polar
