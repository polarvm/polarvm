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

#include "polar/utils/CommandLine.h"
#include "polar/utils/Signals.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#if defined(_WIN32)
# include <windows.h>
# if defined(_MSC_VER)
#   include <crtdbg.h>
# endif
#endif

const char *TestMainArgv0;

int main(int argc, char **argv)
{
   polar::sys::print_stack_trace_on_error_signal(argv[0],
         true /* Disable crash reporting */);

   // Initialize both gmock and gtest.
   testing::InitGoogleMock(&argc, argv);

   polar::cmd::parse_command_line_options(argc, argv);

   // Make it easy for a test to re-execute itself by saving argv[0].
   TestMainArgv0 = argv[0];

# if defined(_WIN32)
   // Disable all of the possible ways Windows conspires to make automated
   // testing impossible.
   ::SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
#   if defined(_MSC_VER)
   ::_set_error_mode(_OUT_TO_STDERR);
   _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
   _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
   _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
   _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
   _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
   _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
#   endif
# endif

   return RUN_ALL_TESTS();
}
