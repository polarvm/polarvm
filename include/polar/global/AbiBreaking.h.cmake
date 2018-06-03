// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/06/03.

#ifndef POLAR_GLOBAL_ABI_BREAKING_CHECKS_H
#define POLAR_GLOBAL_ABI_BREAKING_CHECKS_H

/* Define to enable checks that alter the polarPHP C++ ABI */
#cmakedefine01 POLAR_ENABLE_ABI_BREAKING_CHECKS

/* Define to enable reverse iteration of unordered polar containers */
#cmakedefine01 POLAR_ENABLE_REVERSE_ITERATION

/* Allow selectively disabling link-time mismatch checking so that header-only
   ADT content from polarphp can be used without linking libUtils. */
#if !POLAR_DISABLE_ABI_BREAKING_CHECKS_ENFORCING

// ABI_BREAKING_CHECKS protection: provides link-time failure when clients build
// mismatch with polarPHP
#if defined(_MSC_VER)
// Use pragma with MSVC
#define POLAR_XSTR(s) POLAR_STR(s)
#define POLAR_STR(s) #s
#pragma detect_mismatch("POLAR_ENABLE_ABI_BREAKING_CHECKS", POLAR_XSTR(POLAR_ENABLE_ABI_BREAKING_CHECKS))
#undef POLAR_XSTR
#undef POLAR_STR
#elif defined(_WIN32) || defined(__CYGWIN__) // Win32 w/o #pragma detect_mismatch
// FIXME: Implement checks without weak.
#elif defined(__cplusplus)
namespace polar {

#if POLAR_ENABLE_ABI_BREAKING_CHECKS
extern int g_enableABIBreakingChecks;
__attribute__((weak, visibility ("hidden"))) int *g_verifyEnableABIBreakingChecks = &g_enableABIBreakingChecks;
#else
extern int g_disableABIBreakingChecks;
__attribute__((weak, visibility ("hidden"))) int *g_verifyDisableABIBreakingChecks = &g_disableABIBreakingChecks;
#endif
} // polar

#endif // _MSC_VER

#endif

#endif // POLAR_GLOBAL_ABI_BREAKING_CHECKS_H
