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
//
//  This file implements the operating system Process concept.
//
//===----------------------------------------------------------------------===//

#include "polar/utils/Process.h"
#include "polar/basic/adt/StlExtras.h"
#include "polar/basic/adt/StringExtras.h"
#include "polar/global/PolarConfig.h"
#include "polar/utils/FileSystem.h"
#include "polar/utils/Path.h"
#include "polar/utils/Program.h"
