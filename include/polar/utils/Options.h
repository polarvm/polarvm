// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/06/11.

/// \file
/// This file declares helper objects for defining debug options that can be
/// configured via the command line. The new API currently builds on the cl::opt
/// API, but does not require the use of static globals.
///
/// With this API options are registered during initialization. For passes, this
/// happens during pass initialization. Passes with options will call a static
/// registerOptions method during initialization that registers options with the
/// OptionRegistry. An example implementation of registerOptions is:
///
/// static void registerOptions() {
///   OptionRegistry::registerOption<bool, Scalarizer,
///                                &Scalarizer::ScalarizeLoadStore>(
///       "scalarize-load-store",
///       "Allow the scalarizer pass to scalarize loads and store", false);
/// }
///
/// When reading data for options the interface is via the LLVMContext. Option
/// data for passes should be read from the context during doInitialization. An
/// example of reading the above option would be:
///
/// ScalarizeLoadStore =
///   M.getContext().getOption<bool,
///                            Scalarizer,
///                            &Scalarizer::ScalarizeLoadStore>();
///

#ifndef POLAR_UTILS_OPTIONS_H
#define POLAR_UTILS_OPTIONS_H

#include "polar/basic/adt/DenseMap.h"
#include "polar/utils/CommandLine.h"


#endif // POLAR_UTILS_OPTIONS_H
