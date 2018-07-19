<%header

%>
<%!
this->setOutputFile("include/polar/syntax/TypeDefs.h");
%>
// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/07/12.

#ifndef POLAR_UNITTEST_SUPPORT_HELPER_H
#define POLAR_UNITTEST_SUPPORT_HELPER_H

#include "polar/basic/adt/StringRef.h"
#include "polar/utils/ErrorType.h"
#include "gtest/gtest-printers.h"

namespace polar {

<% for (int i = 0; i < 100; i++) { %>
TOKEN(name, <%= i %>, <%= i + 1 %>);
<% } %>

} // polar

#endif // POLAR_UNITTEST_SUPPORT_HELPER_H
