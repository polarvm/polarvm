// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/07/03.

//===----------------------------------------------------------------------===//
//
// This file implements the helper objects for defining debug options using the
// new API built on cmd::Opt, but not requiring the use of static globals.
//
//===----------------------------------------------------------------------===//

#include "polar/utils/Options.h"
#include "polar/global/ManagedStatic.h"

namespace polar {
namespace utils {

OptionRegistry::~OptionRegistry()
{
   for (auto iter = m_options.begin(); iter != m_options.end(); ++iter) {
      delete iter->second;
   }
}

void OptionRegistry::addOption(void *key, cmd::Option *option)
{
   assert(m_options.find(key) == m_options.end() &&
          "Argument with this key already registerd");
   m_options.insert(std::make_pair(key, option));
}

static ManagedStatic<OptionRegistry> sg_optionRegistry;

OptionRegistry &OptionRegistry::instance()
{
   return *sg_optionRegistry;
}

} // utils
} // polar
