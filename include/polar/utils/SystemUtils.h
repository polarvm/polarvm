// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/07/04.

#ifndef POLAR_UTILS_SYSTEM_UTILS_H
#define POLAR_UTILS_SYSTEM_UTILS_H

class RawOutStream;

/// Determine if the raw_ostream provided is connected to a terminal. If so,
/// generate a warning message to errs() advising against display of bitcode
/// and return true. Otherwise just return false.
/// @brief Check for output written to a console
bool check_bitcode_output_to_console(
      RawOutStream &streamToCheck, ///< The stream to be checked
      bool printWarning = true     ///< Control whether warnings are printed
      );

#endif // POLAR_UTILS_SYSTEM_UTILS_H
