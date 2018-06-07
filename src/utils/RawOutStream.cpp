// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/06/07.

//===----------------------------------------------------------------------===//
//
// This implements support for bulk buffered stream output.
//
//===----------------------------------------------------------------------===//

#include "polar/utils/RawOutStream.h"
#include "polar/basic/adt/StlExtras.h"
#include "polar/basic/adt/SmallVector.h"
#include "polar/basic/adt/StringExtras.h"
#include "polar/global/Global.h"
#include "polar/utils/ErrorHandling.h"
#include "polar/utils/FileSystem.h"
#include "polar/utils/Format.h"
#include "polar/utils/FormatVariadic.h"
#include "polar/utils/MathExtras.h"
#include "polar/utils/NativeFormatting.h"
#include "polar/utils/Process.h"
#include "polar/utils/Program.h"
#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstdio>
#include <iterator>
#include <sys/stat.h>
#include <system_error>
