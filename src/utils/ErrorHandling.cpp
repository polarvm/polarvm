// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/06/02.

#include "polar/utils/ErrorHandling.h"

namespace polar {
namespace utils {

void report_bad_alloc_error(const char *reason, bool genCrashDiag)
{}

void report_fatal_error(const char *reason, bool genCrashDiag)
{
}

void unreachable_internal(const char *msg, const char *file,
                          unsigned line)
{
}

} // utils
} // polar
