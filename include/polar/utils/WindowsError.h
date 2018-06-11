// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/06/09.

#ifndef POLAR_UTILS_WINDOWS_ERROR_H
#define POLAR_UTILS_WINDOWS_ERROR_H

#include <system_error>

namespace polar {
namespace utils {

std::error_code map_windows_error(unsigned ev);

} // utils
} // polar

#endif // POLAR_UTILS_WINDOWS_ERROR_H
