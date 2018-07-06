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
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

#include "polar/utils/program.h"
#include "polar/basic/adt/StringRef.h"
#include "polar/global/PolarConfig.h"
#include <system_error>

namespace polar {
namespace sys {

bool execute(ProcessInfo &processInfo, StringRef program, const char **args,
             const char **envp, ArrayRef<std::optional<StringRef>> redirects,
             unsigned memoryLimit, std::string *errMsg);

int execute_and_wait(StringRef program, const char **args, const char **envp,
                     ArrayRef<std::optional<StringRef>> redirects,
                     unsigned secondsToWait, unsigned memoryLimit,
                     std::string *errMsg, bool *executionFailed)
{
   assert(redirects.empty() || redirects.getSize() == 3);
   ProcessInfo processInfo;
   if (execute(processInfo, program, args, envp, redirects, memoryLimit, errMsg)) {
      if (executionFailed) {
         *executionFailed = false;
      }
      ProcessInfo result = wait(
               processInfo, secondsToWait, /*WaitUntilTerminates=*/secondsToWait == 0, errMsg);
      return result.m_returnCode;
   }

   if (executionFailed) {
      *executionFailed = true;
   }
   return -1;
}

ProcessInfo execute_no_wait(StringRef program, const char **args,
                            const char **envp,
                            ArrayRef<std::optional<StringRef>> redirects,
                            unsigned memoryLimit, std::string *errMsg,
                            bool *executionFailed)
{
   assert(redirects.empty() || redirects.getSize() == 3);
   ProcessInfo processInfo;
   if (executionFailed) {
       *executionFailed = false;
   }
   if (!execute(processInfo, program, args, envp, redirects, memoryLimit, errMsg)) {
      if (executionFailed) {
         *executionFailed = true;
      }
   }
   return processInfo;
}

} // sys
} // polar
