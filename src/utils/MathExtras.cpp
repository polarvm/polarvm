// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/06/04.

#include "polar/utils/MathExtras.h"

#ifdef POLAR_CC_MSVC
#include <limits>
#else
#include <math.h>
#endif

namespace polar {
namespace utils {

#if defined(POLAR_CC_MSVC)
// Visual Studio defines the HUGE_VAL class of macros using purposeful
// constant arithmetic overflow, which it then warns on when encountered.
const float g_hugeValf = std::numeric_limits<float>::infinity();
#else
const float g_hugeValf = HUGE_VALF;
#endif

} // utils
} // polar
