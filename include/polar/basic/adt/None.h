// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
// 
// Created by softboy on 2018/05/27.

#ifndef POLAR_BASIC_ADT_NONE_H
#define POLAR_BASIC_ADT_NONE_H

namespace polar {
namespace basic {

/// \brief A simple null object to allow implicit construction of Optional<T>
/// and similar types without having to spell out the specialization's name.
// (constant value 1 in an attempt to workaround MSVC build issue... )
enum class NoneType
{
   None = 1
};

const NoneType None = NoneType::None;

} // basic
} // polar

#endif // POLAR_BASIC_ADT_NONE_H
