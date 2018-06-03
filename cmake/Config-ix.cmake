if(WIN32 AND NOT CYGWIN)
   # We consider Cygwin as another Unix
   set(PURE_WINDOWS 1)
endif()

include(CheckIncludeFile)
include(CheckIncludeFileCXX)
include(CheckLibraryExists)
include(CheckSymbolExists)
include(CheckFunctionExists)
include(CheckCCompilerFlag)
include(CheckCXXSourceCompiles)
include(TestBigEndian)
include(CheckCompilerVersion)

if(UNIX AND NOT (BEOS OR HAIKU))
   # Used by check_symbol_exists:
   list(APPEND CMAKE_REQUIRED_LIBRARIES "m")
endif()

# x86_64 FreeBSD 9.2 requires libcxxrt to be specified explicitly.
if(CMAKE_SYSTEM MATCHES "FreeBSD-9.2-RELEASE" AND
      CMAKE_SIZEOF_VOID_P EQUAL 8 )
   list(APPEND CMAKE_REQUIRED_LIBRARIES "cxxrt")
endif()

# Helper macros and functions
macro(polar_add_cxx_include result files)
   set(${result} "")
   foreach (file_name ${files})
      set(${result} "${${result}}#include<${file_name}>\n")
   endforeach()
endmacro()

function(polar_check_type_exists type files variable)
   polar_add_cxx_include(includes "${files}")
   CHECK_CXX_SOURCE_COMPILES("
      ${includes} ${type} typeVar;
      int main() {
      return 0;
      }
      " ${variable})
endfunction()

# include checks
check_include_file(dirent.h HAVE_DIRENT_H)
check_include_file(dlfcn.h HAVE_DLFCN_H)
check_include_file(errno.h HAVE_ERRNO_H)
check_include_file(fcntl.h HAVE_FCNTL_H)
check_include_file(inttypes.h HAVE_INTTYPES_H)
check_include_file(link.h HAVE_LINK_H)
check_include_file(malloc.h HAVE_MALLOC_H)
check_include_file(malloc/malloc.h HAVE_MALLOC_MALLOC_H)
check_include_file(ndir.h HAVE_NDIR_H)

if(NOT PURE_WINDOWS)
   check_include_file(pthread.h HAVE_PTHREAD_H)
endif()

check_include_file(signal.h HAVE_SIGNAL_H)
check_include_file(stdint.h HAVE_STDINT_H)
check_include_file(sys/dir.h HAVE_SYS_DIR_H)
check_include_file(sys/ioctl.h HAVE_SYS_IOCTL_H)
check_include_file(sys/mman.h HAVE_SYS_MMAN_H)
check_include_file(sys/ndir.h HAVE_SYS_NDIR_H)
check_include_file(sys/param.h HAVE_SYS_PARAM_H)
check_include_file(sys/resource.h HAVE_SYS_RESOURCE_H)
check_include_file(sys/stat.h HAVE_SYS_STAT_H)
check_include_file(sys/time.h HAVE_SYS_TIME_H)
check_include_file(sys/types.h HAVE_SYS_TYPES_H)
check_include_file(sys/uio.h HAVE_SYS_UIO_H)
check_include_file(termios.h HAVE_TERMIOS_H)
check_include_file(unistd.h HAVE_UNISTD_H)
check_include_file(valgrind/valgrind.h HAVE_VALGRIND_VALGRIND_H)
check_include_file(zlib.h HAVE_ZLIB_H)
check_include_file(fenv.h HAVE_FENV_H)
check_symbol_exists(FE_ALL_EXCEPT "fenv.h" HAVE_DECL_FE_ALL_EXCEPT)
check_symbol_exists(FE_INEXACT "fenv.h" HAVE_DECL_FE_INEXACT)
check_include_file(mach/mach.h HAVE_MACH_MACH_H)
check_include_file(histedit.h HAVE_HISTEDIT_H)
check_include_file(CrashReporterClient.h HAVE_CRASHREPORTERCLIENT_H)
if(APPLE)
   include(CheckCSourceCompiles)
   CHECK_C_SOURCE_COMPILES("
      static const char *__crashreporter_info__ = 0;
      asm(\".desc ___crashreporter_info__, 0x10\");
      int main() { return 0; }"
      HAVE_CRASHREPORTER_INFO)
endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  check_include_file(linux/magic.h HAVE_LINUX_MAGIC_H)
  if(NOT HAVE_LINUX_MAGIC_H)
    # older kernels use split files
    check_include_file(linux/nfs_fs.h HAVE_LINUX_NFS_FS_H)
    check_include_file(linux/smb.h HAVE_LINUX_SMB_H)
  endif()
endif()

# library checks
if( NOT PURE_WINDOWS )
   check_library_exists(pthread pthread_create "" HAVE_LIBPTHREAD)
   if (HAVE_LIBPTHREAD)
      check_library_exists(pthread pthread_getspecific "" HAVE_PTHREAD_GETSPECIFIC)
      check_library_exists(pthread pthread_rwlock_init "" HAVE_PTHREAD_RWLOCK_INIT)
      check_library_exists(pthread pthread_mutex_lock "" HAVE_PTHREAD_MUTEX_LOCK)
   else()
      # this could be Android
      check_library_exists(c pthread_create "" PTHREAD_IN_LIBC)
      if (PTHREAD_IN_LIBC)
         check_library_exists(c pthread_getspecific "" HAVE_PTHREAD_GETSPECIFIC)
         check_library_exists(c pthread_rwlock_init "" HAVE_PTHREAD_RWLOCK_INIT)
         check_library_exists(c pthread_mutex_lock "" HAVE_PTHREAD_MUTEX_LOCK)
      endif()
   endif()
   check_library_exists(dl dlopen "" HAVE_LIBDL)
   check_library_exists(rt clock_gettime "" HAVE_LIBRT)
endif()

if(HAVE_LIBPTHREAD)
  # We want to find pthreads library and at the moment we do want to
  # have it reported as '-l<lib>' instead of '-pthread'.
  # TODO: switch to -pthread once the rest of the build system can deal with it.
  set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
  set(THREADS_HAVE_PTHREAD_ARG Off)
  find_package(Threads REQUIRED)
  set(POLAR_PTHREAD_LIB ${CMAKE_THREAD_LIBS_INIT})
endif()

# function checks
check_symbol_exists(arc4random "stdlib.h" HAVE_DECL_ARC4RANDOM)
find_package(Backtrace)
set(HAVE_BACKTRACE ${Backtrace_FOUND})
set(BACKTRACE_HEADER ${Backtrace_HEADER})

# Prevent check_symbol_exists from using API that is not supported for a given
# deployment target.
check_c_compiler_flag("-Werror=unguarded-availability-new" "C_SUPPORTS_WERROR_UNGUARDED_AVAILABILITY_NEW")
if(C_SUPPORTS_WERROR_UNGUARDED_AVAILABILITY_NEW)
  set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -Werror=unguarded-availability-new")
endif()

check_symbol_exists(_Unwind_Backtrace "unwind.h" HAVE__UNWIND_BACKTRACE)
check_symbol_exists(getpagesize unistd.h HAVE_GETPAGESIZE)
check_symbol_exists(sysconf unistd.h HAVE_SYSCONF)
check_symbol_exists(getrusage sys/resource.h HAVE_GETRUSAGE)
check_symbol_exists(setrlimit sys/resource.h HAVE_SETRLIMIT)
check_symbol_exists(isatty unistd.h HAVE_ISATTY)
check_symbol_exists(futimens sys/stat.h HAVE_FUTIMENS)
check_symbol_exists(futimes sys/time.h HAVE_FUTIMES)
check_symbol_exists(posix_fallocate fcntl.h HAVE_POSIX_FALLOCATE)

# AddressSanitizer conflicts with lib/Support/Unix/Signals.inc
# Avoid sigaltstack on Apple platforms, where backtrace() cannot handle it
# (rdar://7089625) and _Unwind_Backtrace is unusable because it cannot unwind
# past the signal handler after an assertion failure (rdar://29866587).
if( HAVE_SIGNAL_H AND NOT POLAR_USE_SANITIZER MATCHES ".*Address.*" AND NOT APPLE )
   check_symbol_exists(sigaltstack signal.h HAVE_SIGALTSTACK)
endif()
if( HAVE_SYS_UIO_H )
   check_symbol_exists(writev sys/uio.h HAVE_WRITEV)
endif()
set(CMAKE_REQUIRED_DEFINITIONS "-D_LARGEFILE64_SOURCE")
check_symbol_exists(lseek64 "sys/types.h;unistd.h" HAVE_LSEEK64)
set(CMAKE_REQUIRED_DEFINITIONS "")
check_symbol_exists(mallctl malloc_np.h HAVE_MALLCTL)
check_symbol_exists(mallinfo malloc.h HAVE_MALLINFO)
check_symbol_exists(malloc_zone_statistics malloc/malloc.h
   HAVE_MALLOC_ZONE_STATISTICS)
check_symbol_exists(mkdtemp "stdlib.h;unistd.h" HAVE_MKDTEMP)
check_symbol_exists(mkstemp "stdlib.h;unistd.h" HAVE_MKSTEMP)
check_symbol_exists(mktemp "stdlib.h;unistd.h" HAVE_MKTEMP)
check_symbol_exists(getcwd unistd.h HAVE_GETCWD)
check_symbol_exists(gettimeofday sys/time.h HAVE_GETTIMEOFDAY)
check_symbol_exists(getrlimit "sys/types.h;sys/time.h;sys/resource.h" HAVE_GETRLIMIT)
check_symbol_exists(posix_spawn spawn.h HAVE_POSIX_SPAWN)
check_symbol_exists(pread unistd.h HAVE_PREAD)
check_symbol_exists(realpath stdlib.h HAVE_REALPATH)
check_symbol_exists(sbrk unistd.h HAVE_SBRK)
check_symbol_exists(strtoll stdlib.h HAVE_STRTOLL)
check_symbol_exists(strerror string.h HAVE_STRERROR)
check_symbol_exists(strerror_r string.h HAVE_STRERROR_R)
check_symbol_exists(strerror_s string.h HAVE_DECL_STRERROR_S)
check_symbol_exists(setenv stdlib.h HAVE_SETENV)
if( PURE_WINDOWS )
   check_symbol_exists(_chsize_s io.h HAVE__CHSIZE_S)
   
   check_function_exists(_alloca HAVE__ALLOCA)
   check_function_exists(__alloca HAVE___ALLOCA)
   check_function_exists(__chkstk HAVE___CHKSTK)
   check_function_exists(__chkstk_ms HAVE___CHKSTK_MS)
   check_function_exists(___chkstk HAVE____CHKSTK)
   check_function_exists(___chkstk_ms HAVE____CHKSTK_MS)
   
   check_function_exists(__ashldi3 HAVE___ASHLDI3)
   check_function_exists(__ashrdi3 HAVE___ASHRDI3)
   check_function_exists(__divdi3 HAVE___DIVDI3)
   check_function_exists(__fixdfdi HAVE___FIXDFDI)
   check_function_exists(__fixsfdi HAVE___FIXSFDI)
   check_function_exists(__floatdidf HAVE___FLOATDIDF)
   check_function_exists(__lshrdi3 HAVE___LSHRDI3)
   check_function_exists(__moddi3 HAVE___MODDI3)
   check_function_exists(__udivdi3 HAVE___UDIVDI3)
   check_function_exists(__umoddi3 HAVE___UMODDI3)
   
   check_function_exists(__main HAVE___MAIN)
   check_function_exists(__cmpdi2 HAVE___CMPDI2)
endif()

if(HAVE_DLFCN_H)
  if(HAVE_LIBDL)
    list(APPEND CMAKE_REQUIRED_LIBRARIES dl)
  endif()
  check_symbol_exists(dlopen dlfcn.h HAVE_DLOPEN)
  check_symbol_exists(dladdr dlfcn.h HAVE_DLADDR)
  if(HAVE_LIBDL)
    list(REMOVE_ITEM CMAKE_REQUIRED_LIBRARIES dl)
  endif()
endif()

check_symbol_exists(__GLIBC__ stdio.h POLAR_USING_GLIBC)
if(POLAR_USING_GLIBC)
   add_definitions( -D_GNU_SOURCE )
   list(APPEND CMAKE_REQUIRED_DEFINITIONS "-D_GNU_SOURCE")
endif()
# This check requires _GNU_SOURCE
check_symbol_exists(sched_getaffinity sched.h HAVE_SCHED_GETAFFINITY)
check_symbol_exists(CPU_COUNT sched.h HAVE_CPU_COUNT)
if(HAVE_LIBPTHREAD)
   check_library_exists(pthread pthread_getname_np "" HAVE_PTHREAD_GETNAME_NP)
   check_library_exists(pthread pthread_setname_np "" HAVE_PTHREAD_SETNAME_NP)
elseif(PTHREAD_IN_LIBC)
   check_library_exists(c pthread_getname_np "" HAVE_PTHREAD_GETNAME_NP)
   check_library_exists(c pthread_setname_np "" HAVE_PTHREAD_SETNAME_NP)
endif()

set(headers "sys/types.h")

if (HAVE_INTTYPES_H)
   set(headers ${headers} "inttypes.h")
endif()

if (HAVE_STDINT_H)
   set(headers ${headers} "stdint.h")
endif()

polar_check_type_exists(int64_t "${headers}" HAVE_INT64_T)
polar_check_type_exists(uint64_t "${headers}" HAVE_UINT64_T)
polar_check_type_exists(u_int64_t "${headers}" HAVE_U_INT64_T)

# available programs checks
function(polar_find_program name)
   string(TOUPPER ${name} NAME)
   string(REGEX REPLACE "\\." "_" NAME ${NAME})
   
   find_program(POLAR_PATH_${NAME} NAMES ${ARGV})
   mark_as_advanced(POLAR_PATH_${NAME})
   if(POLAR_PATH_${NAME})
      set(HAVE_${NAME} 1 CACHE INTERNAL "Is ${name} available ?")
      mark_as_advanced(HAVE_${NAME})
   else(POLAR_PATH_${NAME})
      set(HAVE_${NAME} "" CACHE INTERNAL "Is ${name} available ?")
   endif(POLAR_PATH_${NAME})
endfunction()

if (POLAR_ENABLE_DOXYGEN)
   polar_find_program(dot)
endif ()

check_cxx_compiler_flag("-Wvariadic-macros" SUPPORTS_VARIADIC_MACROS_FLAG)
check_cxx_compiler_flag("-Wgnu-zero-variadic-macro-arguments"
                        SUPPORTS_GNU_ZERO_VARIADIC_MACRO_ARGUMENTS_FLAG)

set(USE_NO_MAYBE_UNINITIALIZED 0)
set(USE_NO_UNINITIALIZED 0)

# Disable gcc's potentially uninitialized use analysis as it presents lots of
# false positives.
if (CMAKE_COMPILER_IS_GNUCXX)
   check_cxx_compiler_flag("-Wmaybe-uninitialized" HAS_MAYBE_UNINITIALIZED)
   if (HAS_MAYBE_UNINITIALIZED)
      set(USE_NO_MAYBE_UNINITIALIZED 1)
   else()
      # Only recent versions of gcc make the distinction between -Wuninitialized
      # and -Wmaybe-uninitialized. If -Wmaybe-uninitialized isn't supported, just
      # turn off all uninitialized use warnings.
      check_cxx_compiler_flag("-Wuninitialized" HAS_UNINITIALIZED)
      set(USE_NO_UNINITIALIZED ${HAS_UNINITIALIZED})
   endif()
endif()

# By default, we target the host, but this can be overridden at CMake
# invocation time.
include(GetHostTriple)
polar_get_host_triple(POLAR_INFERRED_HOST_TRIPLE)

set(POLAR_HOST_TRIPLE "${POLAR_INFERRED_HOST_TRIPLE}" CACHE STRING
   "Host on which POLAR binaries will run")

# Determine the native architecture.
string(TOLOWER "${POLAR_TARGET_ARCH}" POLAR_NATIVE_ARCH)
if(POLAR_NATIVE_ARCH STREQUAL "host" )
   string(REGEX MATCH "^[^-]*" POLAR_NATIVE_ARCH ${POLAR_HOST_TRIPLE})
endif ()

message(${POLAR_NATIVE_ARCH})

if (POLAR_NATIVE_ARCH MATCHES "i[2-6]86")
   set(POLAR_NATIVE_ARCH X86)
elseif (POLAR_NATIVE_ARCH STREQUAL "x86")
   set(POLAR_NATIVE_ARCH X86)
elseif (POLAR_NATIVE_ARCH STREQUAL "amd64")
   set(POLAR_NATIVE_ARCH X86)
elseif (POLAR_NATIVE_ARCH STREQUAL "x86_64")
   set(POLAR_NATIVE_ARCH X86)
elseif (POLAR_NATIVE_ARCH MATCHES "sparc")
   set(POLAR_NATIVE_ARCH Sparc)
elseif (POLAR_NATIVE_ARCH MATCHES "powerpc")
   set(POLAR_NATIVE_ARCH PowerPC)
elseif (POLAR_NATIVE_ARCH MATCHES "aarch64")
   set(POLAR_NATIVE_ARCH AArch64)
elseif (POLAR_NATIVE_ARCH MATCHES "arm64")
   set(POLAR_NATIVE_ARCH AArch64)
elseif (POLAR_NATIVE_ARCH MATCHES "arm")
   set(POLAR_NATIVE_ARCH ARM)
elseif (POLAR_NATIVE_ARCH MATCHES "mips")
   set(POLAR_NATIVE_ARCH Mips)
elseif (POLAR_NATIVE_ARCH MATCHES "xcore")
   set(POLAR_NATIVE_ARCH XCore)
elseif (POLAR_NATIVE_ARCH MATCHES "msp430")
   set(POLAR_NATIVE_ARCH MSP430)
elseif (POLAR_NATIVE_ARCH MATCHES "hexagon")
   set(POLAR_NATIVE_ARCH Hexagon)
elseif (POLAR_NATIVE_ARCH MATCHES "s390x")
   set(POLAR_NATIVE_ARCH SystemZ)
elseif (POLAR_NATIVE_ARCH MATCHES "wasm32")
   set(POLAR_NATIVE_ARCH WebAssembly)
elseif (POLAR_NATIVE_ARCH MATCHES "wasm64")
   set(POLAR_NATIVE_ARCH WebAssembly)
else ()
   message(FATAL_ERROR "Unknown architecture ${POLAR_NATIVE_ARCH}")
endif ()

if(MINGW)
   set(HAVE_LIBPSAPI 1)
   set(HAVE_LIBSHELL32 1)
   # TODO: Check existence of libraries.
   #   include(CheckLibraryExists)
endif(MINGW)

if (NOT HAVE_STRTOLL)
   # Use _strtoi64 if strtoll is not available.
   check_symbol_exists(_strtoi64 stdlib.h have_strtoi64)
   if (have_strtoi64)
      set(HAVE_STRTOLL 1)
      set(strtoll "_strtoi64")
      set(strtoull "_strtoui64")
   endif ()
endif ()

if(CMAKE_HOST_APPLE AND APPLE)
   if(NOT CMAKE_XCRUN)
      find_program(CMAKE_XCRUN NAMES xcrun)
   endif()
   if(CMAKE_XCRUN)
      execute_process(COMMAND ${CMAKE_XCRUN} -find ld
         OUTPUT_VARIABLE LD64_EXECUTABLE
         OUTPUT_STRIP_TRAILING_WHITESPACE)
   else()
      find_program(LD64_EXECUTABLE NAMES ld DOC "The ld64 linker")
   endif()
   
   if(LD64_EXECUTABLE)
      set(LD64_EXECUTABLE ${LD64_EXECUTABLE} CACHE PATH "ld64 executable")
      message(STATUS "Found ld64 - ${LD64_EXECUTABLE}")
   endif()
endif()
