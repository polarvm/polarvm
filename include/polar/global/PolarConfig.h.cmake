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

#ifndef POLAR_GLOABL_POLAR_CONFIG_H
#define POLAR_GLOABL_POLAR_CONFIG_H

/* Host triple POLAR will be executed on */
#cmakedefine POLAR_HOST_TRIPLE "${POLAR_HOST_TRIPLE}"

/* POLAR architecture name for the native architecture, if available */
#cmakedefine POLAR_NATIVE_ARCH "${POLAR_NATIVE_ARCH}"

/* Define if this is Unixish platform */
#cmakedefine POLAR_ON_UNIX ${POLAR_ON_UNIX}

/* Define if this is Win32ish platform */
#cmakedefine POLAR_ON_WIN32 ${POLAR_ON_WIN32}

#define POLAR_PACKAGE_NAME "${POLAR_PACKAGE_NAME}"

/* Major version of the polarphp API */
#define POLAR_VERSION_MAJOR ${POLAR_VERSION_MAJOR}

/* Minor version of the polarphp API */
#define POLAR_VERSION_MINOR ${POLAR_VERSION_MINOR}

/* Patch version of the polarphp API */
#define POLAR_VERSION_PATCH ${POLAR_VERSION_PATCH}

/* polarphp version string */
#define POLAR_VERSION_STRING "${POLAR_PACKAGE_VERSION}"

#endif // POLAR_GLOABL_POLAR_CONFIG_H
