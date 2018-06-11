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

#include "polar/utils/StringSaver.h"

namespace polar {
namespace utils {

StringRef StringSaver::save(StringRef str)
{
  char *ptr = m_alloc.allocate<char>(str.getSize() + 1);
  memcpy(ptr, str.getData(), str.getSize());
  ptr[str.getSize()] = '\0';
  return StringRef(ptr, str.getSize());
}

} // utils
} // polar
