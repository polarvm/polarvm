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

#ifndef POLAR_GLOBAL_GLOBAL_H
#define POLAR_GLOBAL_GLOBAL_H

#ifdef __cplusplus
#  include <type_traits>
#  include <cstddef>
#  include <utility>
#  include <algorithm>
#  include <memory>
#endif
#ifndef __ASSEMBLER__
#  include <stddef.h>
#endif

#include "polar/global/Config.h"
#include "polar/global/DataTypes.h"

#define POLAR_STRINGIFY2(x) #x
#define POLAR_STRINGIFY(x) POLAR_STRINGIFY2(x)

#include "polar/global/SystemDetection.h"
#include "polar/global/ProcessorDetection.h"
#include "polar/global/CompilerDetection.h"

#if defined(__ELF__)
#   define POLAR_OF_ELF
#endif

#if defined(__MACH__) && defined(__APPLE__)
#   define POLAR_OF_MACH_O
#endif

#if defined(POLAR_SHARED) || !defined(POLAR_STATIC)
#  ifdef POLAR_STATIC
#     error "Both POLAR_SHARED and POLAR_STATIC defined, pelase check up your cmake options!"
#  endif
#  ifndef POLAR_SHARED
#     define POLAR_SHARED
#  endif
#  if defined(POLAR_BUILD_SELF)
#     define POLAR_CORE_EXPORT POLAR_DECL_IMPORT
#  else
#     define POLAR_CORE_EXPORT POLAR_DECL_EXPORT
#  endif
#else
#  define POLAR_CORE_EXPORT
#endif

#define POLAR_CONFIG(feature) (1/POLAR_FEATURE_##feature == 1)
#define POLAR_REQUIRE_CONFIG(feature) POLAR_STATIC_ASSERT_X(POLAR_FEATURE_##feature == 1, "Required feature " #feature " for file " __FILE__ " not available.")

#define POLAR_DISABLE_COPY(Class)\
   Class(const Class &) = delete;\
   Class &operator=(const Class &) = delete

#if defined(__i386__) || defined(_WIN32)
#  if defined(POLAR_CC_GNU)
#     define POLAR_FASTCALL __attribute__((regparm(3)))
#  elif defined(POLAR_CC_MSVC)
#     define POLAR_FASTCALL __fastcall
#  else
#     define POLAR_FASTCALL
#  endif
#else
#  define POLAR_FASTCALL
#endif
/*
   Avoid "unused parameter" warnings
*/
#define POLAR_UNUSED(x) (void)x

#define POLAR_TERMINATE_ON_EXCEPTION(expr) do { expr; } while (false)

#if !defined(POLAR_NO_DEBUG) && !defined(POLAR_DEBUG)
#  define POLAR_DEBUG
#endif

#if defined(POLAR_CC_GNU) && !defined(__INSURE__)
#  if defined(POLAR_CC_MINGW) && !defined(POLAR_CC_CLANG)
#     define POLAR_ATTRIBUTE_FORMAT_PRINTF(A, B) \
   __attribute__((format(gnu_printf, (A), (B))))
#  else
#     define POLAR_ATTRIBUTE_FORMAT_PRINTF(A, B) \
   __attribute__((format(printf, (A), (B))))
#  endif
#else
#  define POLAR_ATTRIBUTE_FORMAT_PRINTF(A, B)
#endif

#ifndef __ASSEMBLER__
#  ifdef __cplusplus
namespace polar 
{

using longlong = std::int64_t;
using ulonglong = std::uint64_t;

inline void polar_noop(void) {}

} // POLAR

#  endif // __cplusplus

using uchar = unsigned char ;
using ushort = unsigned short;
using uint = unsigned int;
using ulong = unsigned long;

#  ifdef __cplusplus
namespace polar 
{

//#ifndef POLAR_CC_MSVC
//POLAR_NORETURN
//#endif
POLAR_CORE_EXPORT void polar_assert(const char *assertion, const char *file, 
                                    int line) noexcept;

#if !defined(POLAR_ASSERT)
#  if defined(POLAR_NO_DEBUG) && !defined(POLAR_FORCE_ASSERTS)
#     define POLAR_ASSERT(cond) static_cast<void>(false && (cond))
#  else
#     define POLAR_ASSERT(cond) ((cond) ? static_cast<void>(0) : ::polar::polar_assert(#cond,__FILE__,__LINE__))
#  endif
#endif

/*
  uintptr and ptrdiff is guaranteed to be the same size as a pointer, i.e.
  
      sizeof(void *) == sizeof(uintptr)
      && sizeof(void *) == sizeof(ptrdiff)
*/
template <int> struct IntegerForSize;

template <> 
struct IntegerForSize<1>
{
   using Unsigned = std::uint8_t;
   using Signed = std::int8_t;
};

template <>
struct IntegerForSize<2>
{
   using Unsigned = std::uint16_t;
   using Signed = std::int16_t;
};

template <>
struct IntegerForSize<4>
{
   using Unsigned = std::uint32_t;
   using Signed = std::int32_t;
};

template <>
struct IntegerForSize<8>
{
   using Unsigned = std::uint64_t;
   using Signed = std::int64_t;
};

#if defined(POLAR_CC_GNU) && defined(__SIZEOF_INT128__)
template <>
struct IntegerForSize<16>
{
   __extension__ typedef unsigned __int128 Unsigned;
   __extension__ typedef __int128 Signed;
};
#endif

template <typename T>
struct IntegerForSizeof : IntegerForSize<sizeof(T)>
{};

using registerint = typename IntegerForSize<POLAR_PROCESSOR_WORDSIZE>::Signed;
using registeruint = typename IntegerForSize<POLAR_PROCESSOR_WORDSIZE>::Unsigned;
using uintptr = typename IntegerForSizeof<void *>::Unsigned;
using intptr = typename IntegerForSizeof<void *>::Signed;
using ptrdiff = intptr;
using sizetype = typename IntegerForSizeof<std::size_t>::Signed;

template <typename T>
static inline T *get_ptr_helper(T *ptr)
{
   return ptr;
}

template <typename T>
static inline typename T::pointer get_ptr_helper(const T &p)
{
   return p.get();
}

template <typename T>
static inline T *get_ptr_helper(const std::shared_ptr<T> &p)
{
   return p.get();
}

template <typename T>
static inline T *get_ptr_helper(const std::unique_ptr<T> &p)
{
   return p.get();
}

#define POLAR_DECLARE_PRIVATE(Class)\
   inline Class##Private* getImplPtr()\
{\
   return reinterpret_cast<Class##Private *>(polar::get_ptr_helper(m_implPtr));\
}\
   inline const Class##Private* getImplPtr() const\
{\
   return reinterpret_cast<const Class##Private *>(polar::get_ptr_helper(m_implPtr));\
}\
   friend class Class##Private

#define POLAR_DECLARE_PUBLIC(Class)\
   inline Class* getApiPtr()\
{\
   return static_cast<Class *>(m_apiPtr);\
}\
   inline const Class* getApiPtr() const\
{\
   return static_cast<const Class *>(m_apiPtr);\
}\
   friend class Class

#define POLAR_D(Class) Class##Private * const implPtr = getImplPtr()
#define POLAR_Q(Class) Class * const apiPtr = getApiPtr()

//#ifndef POLAR_CC_MSVC
//POLAR_NORETURN
//#endif
POLAR_CORE_EXPORT void polar_assert_x(const char *where, const char *what, 
                                      const char *file, int line) noexcept;

#if !defined(POLAR_ASSERT_X)
#  if defined(POLAR_NO_DEBUG) && !defined(POLAR_FORCE_ASSERTS)
#     define POLAR_ASSERT_X(cond, where, what) do {} while ((false) && (cond))
#  else
#     define POLAR_ASSERT_X(cond, where, what) ((!(cond)) ? polar::polar_assert_x(where, what,__FILE__,__LINE__) : polar::polar_noop())
#  endif
#endif

#define POLAR_STATIC_ASSERT(Condition) static_assert(bool(Condition), #Condition)
#define POLAR_STATIC_ASSERT_X(Condition, Message) static_assert(bool(Condition), Message)

using NoImplicitBoolCast = int;

#define POLAR_CHECK_ALLOC_PTR(ptr) do { if (!(ptr)) throw std::bad_alloc(); } while (false)

template <typename T>
constexpr const T &bound(const T &min, const T &value, const T &max)
{
   return std::max(min, std::min(value, max));
}

// just as std::as_const
template <typename T>
constexpr typename std::add_const<T>::type &as_const(T &value) noexcept
{
   return value;
}

template <typename T>
void as_const(const T &&) = delete;

#  endif // __cplusplus
#endif // __ASSEMBLER__

#if defined(_MSC_VER)
#include <sal.h>
#endif

#ifndef __has_feature
# define __has_feature(x) 0
#endif

#ifndef __has_extension
# define __has_extension(x) 0
#endif

#ifndef __has_attribute
# define __has_attribute(x) 0
#endif

#ifndef __has_cpp_attribute
# define __has_cpp_attribute(x) 0
#endif

#ifndef __has_builtin
# define __has_builtin(x) 0
#endif

/// \macro POLAR_GNUC_PREREQ
/// \brief Extend the default __GNUC_PREREQ even if glibc's features.h isn't
/// available.
#ifndef POLAR_GNUC_PREREQ
# if defined(__GNUC__) && defined(__GNUC_MINOR__) && defined(__GNUC_PATCHLEVEL__)
#  define POLAR_GNUC_PREREQ(maj, min, patch) \
   ((__GNUC__ << 20) + (__GNUC_MINOR__ << 10) + __GNUC_PATCHLEVEL__ >= \
   ((maj) << 20) + ((min) << 10) + (patch))
# elif defined(__GNUC__) && defined(__GNUC_MINOR__)
#  define POLAR_GNUC_PREREQ(maj, min, patch) \
   ((__GNUC__ << 20) + (__GNUC_MINOR__ << 10) >= ((maj) << 20) + ((min) << 10))
# else
#  define POLAR_GNUC_PREREQ(maj, min, patch) 0
# endif
#endif

/// \macro POLAR_MSC_PREREQ
/// \brief Is the compiler MSVC of at least the specified version?
/// The common \param version values to check for are:
///  * 1900: Microsoft Visual Studio 2015 / 14.0
#ifdef _MSC_VER
#define POLAR_MSC_PREREQ(version) (_MSC_VER >= (version))

// We require at least MSVC 2015.
#if !POLAR_MSC_PREREQ(1900)
#error LLVM requires at least MSVC 2015.
#endif

#else
#define POLAR_MSC_PREREQ(version) 0
#endif

/// \brief Does the compiler support ref-qualifiers for *this?
///
/// Sadly, this is separate from just rvalue reference support because GCC
/// and MSVC implemented this later than everything else.
#if __has_feature(cxx_rvalue_references) || POLAR_GNUC_PREREQ(4, 8, 1)
#define POLAR_HAS_RVALUE_REFERENCE_THIS 1
#else
#define POLAR_HAS_RVALUE_REFERENCE_THIS 0
#endif

/// Expands to '&' if ref-qualifiers for *this are supported.
///
/// This can be used to provide lvalue/rvalue overrides of member functions.
/// The rvalue override should be guarded by POLAR_HAS_RVALUE_REFERENCE_THIS
#if POLAR_HAS_RVALUE_REFERENCE_THIS
#define POLAR_LVALUE_FUNCTION &
#else
#define POLAR_LVALUE_FUNCTION
#endif

/// POLAR_LIBRARY_VISIBILITY - If a class marked with this attribute is linked
/// into a shared library, then the class should be private to the library and
/// not accessible from outside it.  Can also be used to mark variables and
/// functions, making them private to any shared library they are linked into.
/// On PE/COFF targets, library visibility is the default, so this isn't needed.
#if (__has_attribute(visibility) || POLAR_GNUC_PREREQ(4, 0, 0)) &&              \
   !defined(__MINGW32__) && !defined(__CYGWIN__) && !defined(POLAR_ON_WIN32)
#define POLAR_LIBRARY_VISIBILITY __attribute__ ((visibility("hidden")))
#else
#define POLAR_LIBRARY_VISIBILITY
#endif

#if defined(__GNUC__)
#define POLAR_PREFETCH(addr, rw, locality) __builtin_prefetch(addr, rw, locality)
#else
#define POLAR_PREFETCH(addr, rw, locality)
#endif

#if __has_attribute(used) || POLAR_GNUC_PREREQ(3, 1, 0)
#define POLAR_ATTRIBUTE_USED __attribute__((__used__))
#else
#define POLAR_ATTRIBUTE_USED
#endif

/// POLAR_NODISCARD - Warn if a type or return value is discarded.
#if __cplusplus > 201402L && __has_cpp_attribute(nodiscard)
#define POLAR_NODISCARD [[nodiscard]]
#elif !__cplusplus
// Workaround for llvm.org/PR23435, since clang 3.6 and below emit a spurious
// error when __has_cpp_attribute is given a scoped attribute in C mode.
#define POLAR_NODISCARD
#elif __has_cpp_attribute(clang::warn_unused_result)
#define POLAR_NODISCARD [[clang::warn_unused_result]]
#else
#define POLAR_NODISCARD
#endif

// Some compilers warn about unused functions. When a function is sometimes
// used or not depending on build settings (e.g. a function only called from
// within "assert"), this attribute can be used to suppress such warnings.
//
// However, it shouldn't be used for unused *variables*, as those have a much
// more portable solution:
//   (void)unused_var_name;
// Prefer cast-to-void wherever it is sufficient.
#if __has_attribute(unused) || POLAR_GNUC_PREREQ(3, 1, 0)
#define POLAR_ATTRIBUTE_UNUSED __attribute__((__unused__))
#else
#define POLAR_ATTRIBUTE_UNUSED
#endif

// FIXME: Provide this for PE/COFF targets.
#if (__has_attribute(weak) || POLAR_GNUC_PREREQ(4, 0, 0)) &&                    \
   (!defined(__MINGW32__) && !defined(__CYGWIN__) && !defined(POLAR_ON_WIN32))
#define POLAR_ATTRIBUTE_WEAK __attribute__((__weak__))
#else
#define POLAR_ATTRIBUTE_WEAK
#endif

// Prior to clang 3.2, clang did not accept any spelling of
// __has_attribute(const), so assume it is supported.
#if defined(__clang__) || defined(__GNUC__)
// aka 'CONST' but following LLVM Conventions.
#define POLAR_READNONE __attribute__((__const__))
#else
#define POLAR_READNONE
#endif

#if __has_attribute(pure) || defined(__GNUC__)
// aka 'PURE' but following LLVM Conventions.
#define POLAR_READONLY __attribute__((__pure__))
#else
#define POLAR_READONLY
#endif

#if __has_builtin(__builtin_expect) || POLAR_GNUC_PREREQ(4, 0, 0)
#define POLAR_LIKELY(EXPR) __builtin_expect((bool)(EXPR), true)
#define POLAR_UNLIKELY(EXPR) __builtin_expect((bool)(EXPR), false)
#else
#define POLAR_LIKELY(EXPR) (EXPR)
#define POLAR_UNLIKELY(EXPR) (EXPR)
#endif

/// POLAR_ATTRIBUTE_NOINLINE - On compilers where we have a directive to do so,
/// mark a method "not for inlining".
#if __has_attribute(noinline) || POLAR_GNUC_PREREQ(3, 4, 0)
#define POLAR_ATTRIBUTE_NOINLINE __attribute__((noinline))
#elif defined(_MSC_VER)
#define POLAR_ATTRIBUTE_NOINLINE __declspec(noinline)
#else
#define POLAR_ATTRIBUTE_NOINLINE
#endif


/// POLAR_ATTRIBUTE_ALWAYS_INLINE - On compilers where we have a directive to do
/// so, mark a method "always inline" because it is performance sensitive. GCC
/// 3.4 supported this but is buggy in various cases and produces unimplemented
/// errors, just use it in GCC 4.0 and later.
#if __has_attribute(always_inline) || POLAR_GNUC_PREREQ(4, 0, 0)
#define POLAR_ATTRIBUTE_ALWAYS_INLINE __attribute__((always_inline))
#define POLAR_ATTRIBUTE_NEVER_INLINE __attribute__((noinline))
#elif defined(POLAR_CC_MSVC)
#define POLAR_ATTRIBUTE_ALWAYS_INLINE __forceinline
#define POLAR_ATTRIBUTE_NEVER_INLINE __declspec(noinline)
#else
#define POLAR_ATTRIBUTE_ALWAYS_INLINE
#endif

#ifdef POLAR_CC_GNU
#define POLAR_ATTRIBUTE_NORETURN __attribute__((noreturn))
#elif defined(POLAR_CC_MSVC)
#define POLAR_ATTRIBUTE_NORETURN __declspec(noreturn)
#else
#define POLAR_ATTRIBUTE_NORETURN
#endif

#if __has_attribute(returns_nonnull) || POLAR_GNUC_PREREQ(4, 9, 0)
#define POLAR_ATTRIBUTE_RETURNS_NONNULL __attribute__((returns_nonnull))
#elif defined(_MSC_VER)
#define POLAR_ATTRIBUTE_RETURNS_NONNULL _Ret_notnull_
#else
#define POLAR_ATTRIBUTE_RETURNS_NONNULL
#endif

/// \macro POLAR_ATTRIBUTE_RETURNS_NOALIAS Used to mark a function as returning a
/// pointer that does not alias any other valid pointer.
#ifdef POLAR_CC_GNU
#define POLAR_ATTRIBUTE_RETURNS_NOALIAS __attribute__((__malloc__))
#elif defined(POLAR_CC_MSVC)
#define POLAR_ATTRIBUTE_RETURNS_NOALIAS __declspec(restrict)
#else
#define POLAR_ATTRIBUTE_RETURNS_NOALIAS
#endif

/// POLAR_FALLTHROUGH - Mark fallthrough cases in switch statements.
#if __cplusplus > 201402L && __has_cpp_attribute(fallthrough)
#define POLAR_FALLTHROUGH [[fallthrough]]
#elif __has_cpp_attribute(gnu::fallthrough)
#define POLAR_FALLTHROUGH [[gnu::fallthrough]]
#elif !__cplusplus
// Workaround for llvm.org/PR23435, since clang 3.6 and below emit a spurious
// error when __has_cpp_attribute is given a scoped attribute in C mode.
#define POLAR_FALLTHROUGH
#elif __has_cpp_attribute(clang::fallthrough)
#define POLAR_FALLTHROUGH [[clang::fallthrough]]
#else
#define POLAR_FALLTHROUGH
#endif

/// POLAR_EXTENSION - Support compilers where we have a keyword to suppress
/// pedantic diagnostics.
#ifdef POLAR_CC_GNU
#define POLAR_EXTENSION __extension__
#else
#define POLAR_EXTENSION
#endif

// POLAR_ATTRIBUTE_DEPRECATED(decl, "message")
#if __has_feature(attribute_deprecated_with_message)
# define POLAR_ATTRIBUTE_DEPRECATED(decl, message) \
   decl __attribute__((deprecated(message)))
#elif defined(POLAR_CC_GNU)
# define POLAR_ATTRIBUTE_DEPRECATED(decl, message) \
   decl __attribute__((deprecated))
#elif defined(POLAR_CC_MSVC)
# define POLAR_ATTRIBUTE_DEPRECATED(decl, message) \
   __declspec(deprecated(message)) decl
#else
# define POLAR_ATTRIBUTE_DEPRECATED(decl, message) \
   decl
#endif

/// POLAR_BUILTIN_UNREACHABLE - On compilers which support it, expands
/// to an expression which states that it is undefined behavior for the
/// compiler to reach this point.  Otherwise is not defined.
#if __has_builtin(__builtin_unreachable) || POLAR_GNUC_PREREQ(4, 5, 0)
# define POLAR_BUILTIN_UNREACHABLE __builtin_unreachable()
#elif defined(_MSC_VER)
# define POLAR_BUILTIN_UNREACHABLE __assume(false)
#endif

/// POLAR_BUILTIN_TRAP - On compilers which support it, expands to an expression
/// which causes the program to exit abnormally.
#if __has_builtin(__builtin_trap) || POLAR_GNUC_PREREQ(4, 3, 0)
# define POLAR_BUILTIN_TRAP __builtin_trap()
#elif defined(_MSC_VER)
// The __debugbreak intrinsic is supported by MSVC, does not require forward
// declarations involving platform-specific typedefs (unlike RaiseException),
// results in a call to vectored exception handlers, and encodes to a short
// instruction that still causes the trapping behavior we want.
# define POLAR_BUILTIN_TRAP __debugbreak()
#else
# define POLAR_BUILTIN_TRAP *(volatile int*)0x11 = 0
#endif

/// POLAR_BUILTIN_DEBUGTRAP - On compilers which support it, expands to
/// an expression which causes the program to break while running
/// under a debugger.
#if __has_builtin(__builtin_debugtrap)
# define POLAR_BUILTIN_DEBUGTRAP __builtin_debugtrap()
#elif defined(_MSC_VER)
// The __debugbreak intrinsic is supported by MSVC and breaks while
// running under the debugger, and also supports invoking a debugger
// when the OS is configured appropriately.
# define POLAR_BUILTIN_DEBUGTRAP __debugbreak()
#else
// Just continue execution when built with compilers that have no
// support. This is a debugging aid and not intended to force the
// program to abort if encountered.
# define POLAR_BUILTIN_DEBUGTRAP
#endif

/// \macro POLAR_ASSUME_ALIGNED
/// \brief Returns a pointer with an assumed alignment.
#if __has_builtin(__builtin_assume_aligned) || POLAR_GNUC_PREREQ(4, 7, 0)
# define POLAR_ASSUME_ALIGNED(p, a) __builtin_assume_aligned(p, a)
#elif defined(POLAR_BUILTIN_UNREACHABLE)
// As of today, clang does not support __builtin_assume_aligned.
# define POLAR_ASSUME_ALIGNED(p, a) \
   (((uintptr_t(p) % (a)) == 0) ? (p) : (POLAR_BUILTIN_UNREACHABLE, (p)))
#else
# define POLAR_ASSUME_ALIGNED(p, a) (p)
#endif

/// \macro POLAR_ALIGNAS
/// \brief Used to specify a minimum alignment for a structure or variable.
#if __GNUC__ && !__has_feature(cxx_alignas) && !POLAR_GNUC_PREREQ(4, 8, 1)
# define POLAR_ALIGNAS(x) __attribute__((aligned(x)))
#else
# define POLAR_ALIGNAS(x) alignas(x)
#endif

/// \macro POLAR_PACKED
/// \brief Used to specify a packed structure.
/// POLAR_PACKED(
///    struct A {
///      int i;
///      int j;
///      int k;
///      long long l;
///   });
///
/// POLAR_PACKED_START
/// struct B {
///   int i;
///   int j;
///   int k;
///   long long l;
/// };
/// POLAR_PACKED_END
#ifdef POLAR_CC_MSVC
# define POLAR_PACKED(d) __pragma(pack(push, 1)) d __pragma(pack(pop))
# define POLAR_PACKED_START __pragma(pack(push, 1))
# define POLAR_PACKED_END   __pragma(pack(pop))
#else
# define POLAR_PACKED(d) d __attribute__((packed))
# define POLAR_PACKED_START _Pragma("pack(push, 1)")
# define POLAR_PACKED_END   _Pragma("pack(pop)")
#endif

/// \macro POLAR_PTR_SIZE
/// \brief A constant integer equivalent to the value of sizeof(void*).
/// Generally used in combination with POLAR_ALIGNAS or when doing computation in
/// the preprocessor.
#ifdef __SIZEOF_POINTER__
# define POLAR_PTR_SIZE __SIZEOF_POINTER__
#elif defined(POLAR_OS_WIN64)
# define POLAR_PTR_SIZE 8
#elif defined(POLAR_OS_WIN32)
# define POLAR_PTR_SIZE 4
#elif defined(POLAR_CC_MSVC)
# error "could not determine POLAR_PTR_SIZE as a constant int for MSVC"
#else
# define POLAR_PTR_SIZE sizeof(void *)
#endif

/// \macro POLAR_MEMORY_SANITIZER_BUILD
/// \brief Whether LLVM itself is built with MemorySanitizer instrumentation.
#if __has_feature(memory_sanitizer)
# define POLAR_MEMORY_SANITIZER_BUILD 1
# include <sanitizer/msan_interface.h>
#else
# define POLAR_MEMORY_SANITIZER_BUILD 0
# define __msan_allocated_memory(p, size)
# define __msan_unpoison(p, size)
#endif

/// \macro POLAR_ADDRESS_SANITIZER_BUILD
/// \brief Whether LLVM itself is built with AddressSanitizer instrumentation.
#if __has_feature(address_sanitizer) || defined(__SANITIZE_ADDRESS__)
# define POLAR_ADDRESS_SANITIZER_BUILD 1
# include <sanitizer/asan_interface.h>
#else
# define POLAR_ADDRESS_SANITIZER_BUILD 0
# define __asan_poison_memory_region(p, size)
# define __asan_unpoison_memory_region(p, size)
#endif

/// \macro POLAR_THREAD_SANITIZER_BUILD
/// \brief Whether LLVM itself is built with ThreadSanitizer instrumentation.
#if __has_feature(thread_sanitizer) || defined(__SANITIZE_THREAD__)
# define POLAR_THREAD_SANITIZER_BUILD 1
#else
# define POLAR_THREAD_SANITIZER_BUILD 0
#endif

#if POLAR_THREAD_SANITIZER_BUILD
// Thread Sanitizer is a tool that finds races in code.
// See http://code.google.com/p/data-race-test/wiki/DynamicAnnotations .
// tsan detects these exact functions by name.
#ifdef __cplusplus
extern "C" {
#endif
void AnnotateHappensAfter(const char *file, int line, const volatile void *cv);
void AnnotateHappensBefore(const char *file, int line, const volatile void *cv);
void AnnotateIgnoreWritesBegin(const char *file, int line);
void AnnotateIgnoreWritesEnd(const char *file, int line);
#ifdef __cplusplus
}
#endif

// This marker is used to define a happens-before arc. The race detector will
// infer an arc from the begin to the end when they share the same pointer
// argument.
# define TsanHappensBefore(cv) AnnotateHappensBefore(__FILE__, __LINE__, cv)

// This marker defines the destination of a happens-before arc.
# define TsanHappensAfter(cv) AnnotateHappensAfter(__FILE__, __LINE__, cv)

// Ignore any races on writes between here and the next TsanIgnoreWritesEnd.
# define TsanIgnoreWritesBegin() AnnotateIgnoreWritesBegin(__FILE__, __LINE__)

// Resume checking for racy writes.
# define TsanIgnoreWritesEnd() AnnotateIgnoreWritesEnd(__FILE__, __LINE__)
#else
# define TsanHappensBefore(cv)
# define TsanHappensAfter(cv)
# define TsanIgnoreWritesBegin()
# define TsanIgnoreWritesEnd()
#endif

/// \macro POLAR_NO_SANITIZE
/// \brief Disable a particular sanitizer for a function.
#if __has_attribute(no_sanitize)
#define POLAR_NO_SANITIZE(KIND) __attribute__((no_sanitize(KIND)))
#else
#define POLAR_NO_SANITIZE(KIND)
#endif

/// \brief Mark debug helper function definitions like dump() that should not be
/// stripped from debug builds.
/// Note that you should also surround dump() functions with
/// `#if !defined(NDEBUG) || defined(POLAR_ENABLE_DUMP)` so they do always
/// get stripped in release builds.
// FIXME: Move this to a private config.h as it's not usable in public headers.
#if !defined(NDEBUG) || defined(POLAR_ENABLE_DUMP)
#define POLAR_DUMP_METHOD POLAR_ATTRIBUTE_NOINLINE POLAR_ATTRIBUTE_USED
#else
#define POLAR_DUMP_METHOD POLAR_ATTRIBUTE_NOINLINE
#endif

/// \macro POLAR_PRETTY_FUNCTION
/// \brief Gets a user-friendly looking function signature for the current scope
/// using the best available method on each platform.  The exact format of the
/// resulting string is implementation specific and non-portable, so this should
/// only be used, for example, for logging or diagnostics.
#if defined(POLAR_CC_MSVC)
#define POLAR_PRETTY_FUNCTION __FUNCSIG__
#elif defined(POLAR_CC_GNU) || defined(POLAR_CC_CLANG)
#define POLAR_PRETTY_FUNCTION __PRETTY_FUNCTION__
#else
#define POLAR_PRETTY_FUNCTION __func__
#endif

#endif // POLAR_GLOBAL_GLOBAL_H
