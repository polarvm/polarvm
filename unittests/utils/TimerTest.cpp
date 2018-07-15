// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/07/15.

#include "polar/utils/Timer.h"
#include "gtest/gtest.h"

#if POLAR_ON_WIN32
#include <windows.h>
#else
#include <time.h>
#endif

using namespace polar;
using namespace polar::basic;
using namespace polar::utils;

namespace {

// FIXME: Put this somewhere in Support, it's also used in LockFileManager.
void sleep_ms()
{
#if POLAR_ON_WIN32
   Sleep(1);
#else
   struct timespec interval;
   interval.tv_sec = 0;
   interval.tv_nsec = 1000000;
   nanosleep(&interval, nullptr);
#endif
}

TEST(TimerTest, testAdditivity)
{
   Timer timer1("timer1", "timer1");

   EXPECT_TRUE(timer1.isInitialized());

   timer1.startTimer();
   timer1.stopTimer();
   auto TR1 = timer1.getTotalTime();

   timer1.startTimer();
   sleep_ms();
   timer1.stopTimer();
   auto TR2 = timer1.getTotalTime();

   EXPECT_TRUE(TR1 < TR2);
}

TEST(TimerTest, testCheckIfTriggered)
{
   Timer timer1("timer1", "timer1");

   EXPECT_FALSE(timer1.hasTriggered());
   timer1.startTimer();
   EXPECT_TRUE(timer1.hasTriggered());
   timer1.stopTimer();
   EXPECT_TRUE(timer1.hasTriggered());

   timer1.clear();
   EXPECT_FALSE(timer1.hasTriggered());
}

} // anonymous namespace
