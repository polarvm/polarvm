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

#ifndef POLAR_UTILS_DEBUG_H
#define POLAR_UTILS_DEBUG_H

namespace polar {
namespace utils {

class RawOutStream;

#ifndef NDEBUG

/// is_current_debug_type - Return true if the specified string is the debug type
/// specified on the command line, or if none was specified on the command line
/// with the -debug-only=X option.
///
bool is_current_debug_type(const char *type);

/// set_current_debug_type - Set the current debug type, as if the -debug-only=X
/// option were specified.  Note that DebugFlag also needs to be set to true for
/// debug output to be produced.
///
void set_current_debug_type(const char *type);

/// set_current_debug_types - Set the current debug type, as if the
/// -debug-only=X,Y,Z option were specified. Note that DebugFlag
/// also needs to be set to true for debug output to be produced.
///
void set_current_debug_types(const char **types, unsigned count);

/// DEBUG_WITH_TYPE macro - This macro should be used by passes to emit debug
/// information.  In the '-debug' option is specified on the commandline, and if
/// this is a debug build, then the code specified as the option to the macro
/// will be executed.  Otherwise it will not be.  Example:
///
/// DEBUG_WITH_TYPE("bitset", dbgs() << "Bitset contains: " << Bitset << "\n");
///
/// This will emit the debug information if -debug is present, and -debug-only
/// is not specified, or is specified as "bitset".
#define DEBUG_WITH_TYPE(TYPE, X)                                        \
   do { if (::polar::utils::g_debugFlag && ::polar::utils::is_current_debug_type(TYPE)) { X; } \
} while (false)

#else
#define is_current_debug_type(X) (false)
#define set_current_debug_type(X)
#define set_current_debug_types(X, N)
#define DEBUG_WITH_TYPE(TYPE, X) do { } while (false)
#endif

/// This boolean is set to true if the '-debug' command line option
/// is specified.  This should probably not be referenced directly, instead, use
/// the DEBUG macro below.
///
extern bool g_debugFlag;

/// \name Verification flags.
///
/// These flags turns on/off that are expensive and are turned off by default,
/// unless macro EXPENSIVE_CHECKS is defined. The flags allow selectively
/// turning the checks on without need to recompile.
/// \{

/// Enables verification of dominator trees.
///
extern bool g_verifyDomInfo;

/// Enables verification of loop info.
///
extern bool g_verifyLoopInfo;

///\}

/// EnableDebugBuffering - This defaults to false.  If true, the debug
/// stream will install signal handlers to dump any buffered debug
/// output.  It allows clients to selectively allow the debug stream
/// to install signal handlers if they are certain there will be no
/// conflict.
///
extern bool g_enableDebugBuffering;

/// debug_stream() - This returns a reference to a raw_ostream for debugging
/// messages.  If debugging is disabled it returns errs().  Use it
/// like: debug_stream() << "foo" << "bar";
RawOutStream &debug_stream();

// DEBUG macro - This macro should be used by passes to emit debug information.
// In the '-debug' option is specified on the commandline, and if this is a
// debug build, then the code specified as the option to the macro will be
// executed.  Otherwise it will not be.  Example:
//
// DEBUG(debug_stream() << "Bitset contains: " << Bitset << "\n");
//
#define DEBUG(X) DEBUG_WITH_TYPE(DEBUG_TYPE, X)

} // utils
} // polar

#endif // POLAR_UTILS_DEBUG_H
