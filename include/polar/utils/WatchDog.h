// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/07/01.

#ifndef POLAR_UTILS_WATCHDOG_H
#define POLAR_UTILS_WATCHDOG_H

#include "polar/global/Global.h"

namespace polar {
namespace sys {

/// This class provides an abstraction for a timeout around an operation
/// that must complete in a given amount of time. Failure to complete before
/// the timeout is an unrecoverable situation and no mechanisms to attempt
/// to handle it are provided.
class WatchDog
{
public:
   WatchDog(unsigned int seconds);
   ~WatchDog();
private:
   // Noncopyable.
   WatchDog(const WatchDog &other) = delete;
   WatchDog &operator=(const WatchDog &other) = delete;
};

} // sys
} // polar

#endif // POLAR_UTILS_WATCHDOG_H
