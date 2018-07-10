// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/07/10.

#include "polar/utils/CommandLine.h"
#include "polar/basic/adt/STLExtras.h"
#include "polar/basic/adt/SmallString.h"
#include "polar/utils/FileSystem.h"
#include "polar/utils/Path.h"
#include "polar/utils/Program.h"
#include "polar/utils/StringSaver.h"
#include "gtest/gtest.h"
#include <fstream>
#include <stdlib.h>
#include <string>

using namespace polar::basic;
using namespace polar::utils;
using namespace polar::cmd;

namespace {

class TempEnvVar {
public:
   TempEnvVar(const char *name, const char *value)
      : name(name) {
      const char *old_value = getenv(name);
      EXPECT_EQ(nullptr, old_value) << old_value;
#if HAVE_SETENV
      setenv(name, value, true);
#else
#   define SKIP_ENVIRONMENT_TESTS
#endif
   }

   ~TempEnvVar() {
#if HAVE_SETENV
      // Assume setenv and unsetenv come together.
      unsetenv(name);
#else
      (void)name; // Suppress -Wunused-private-field.
#endif
   }

private:
   const char *const name;
};

template <typename T>
class StackOption : public Opt<T> {
   typedef Opt<T> Base;
public:
   // One option...
   template<class M0t>
   explicit StackOption(const M0t &M0) : Base(M0) {}

   // Two options...
   template<class M0t, class M1t>
   StackOption(const M0t &M0, const M1t &M1) : Base(M0, M1) {}

   // Three options...
   template<class M0t, class M1t, class M2t>
   StackOption(const M0t &M0, const M1t &M1, const M2t &M2) : Base(M0, M1, M2) {}

   // Four options...
   template<class M0t, class M1t, class M2t, class M3t>
   StackOption(const M0t &M0, const M1t &M1, const M2t &M2, const M3t &M3)
      : Base(M0, M1, M2, M3) {}

   ~StackOption() override { this->removeArgument(); }

   template <class DT> StackOption<T> &operator=(const DT &V) {
      this->setValue(V);
      return *this;
   }
};

class StackSubCommand : public SubCommand {
public:
   StackSubCommand(StringRef Name,
                   StringRef Description = StringRef())
      : SubCommand(Name, Description) {}

   StackSubCommand() : SubCommand() {}

   ~StackSubCommand() { unregisterSubCommand(); }
};


OptionCategory TestCategory("Test Options", "Description");

} // anonymous namespace
