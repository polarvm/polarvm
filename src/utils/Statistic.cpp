// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/06/26.
//===----------------------------------------------------------------------===//
//
// This file implements the 'Statistic' class, which is designed to be an easy
// way to expose various success metrics from passes.  These statistics are
// printed at the end of a run, when the -stats command line option is enabled
// on the command line.
//
// This is useful for reporting information like the number of instructions
// simplified, optimized or removed by various transformations, like this:
//
// static Statistic NumInstEliminated("GCSE", "Number of instructions killed");
//
// Later, in the code: ++NumInstEliminated;
//
//===----------------------------------------------------------------------===//

#include "polar/basic/adt/Statistic.h"
#include "polar/basic/adt/StringExtras.h"
#include "polar/utils/CommandLine.h"
#include "polar/utils/Debug.h"
#include "polar/utils/Format.h"
#include "polar/global/ManagedStatic.h"
#include "polar/utils/Timer.h"
//#include "polar/utils/YamlTraits.h"
#include "polar/utils/RawOutStream.h"
#include <algorithm>
#include <cstring>
