cmake_minimum_required(VERSION 3.11.2)

project(polarphp C CXX ASM)

if(POLICY CMP0068)
   cmake_policy(SET CMP0068 OLD)
endif()
# Add path for custom CMake modules.
list(APPEND CMAKE_MODULE_PATH
   "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
   "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

# Make a job pool for things that can't yet be distributed
cmake_host_system_information(
   RESULT localhost_logical_cores QUERY NUMBER_OF_LOGICAL_CORES)
set_property(GLOBAL PROPERTY JOB_POOLS local_jobs=${localhost_logical_cores})
# Put linking in that category
set_property(GLOBAL PROPERTY JOB_POOL_LINK local_jobs)

# First include general CMake utilities.
include(PolarUtils)
include(CheckSymbolExists)

#
# User-configurable options that control the inclusion and default build
# behavior for components which may not strictly be necessary (tools, examples,
# and tests).
#
# This is primarily to support building smaller or faster project files.
#
option(POLAR_INCLUDE_TOOLS
   "Generate build targets for polarphp tools"
   TRUE)

option(POLAR_BUILD_DYNAMIC_STDLIB
   "Build dynamic variants of the polarphp standard library"
   TRUE)

option(POLAR_BUILD_STATIC_STDLIB
   "Build static variants of the polarphp standard library"
   FALSE)

# In many cases, the CMake build system needs to determine whether to include
# a directory, or perform other actions, based on whether the stdlib is
# being built at all -- statically or dynamically. Please note that these
# flags are not related to the deprecated build-script-impl arguments
# 'build-polarphp-stdlib'. These are not flags that
# the build script should be able to set.
if(POLAR_BUILD_DYNAMIC_STDLIB OR SWIFT_BUILD_STATIC_STDLIB)
   set(POLAR_BUILD_STDLIB TRUE)
else()
   set(POLAR_BUILD_STDLIB FALSE)
endif()

option(POLAR_BUILD_PERF_TESTSUITE
   "Create in-tree targets for building polarphp performance benchmarks."
   FALSE)

option(POLAR_BUILD_EXTERNAL_PERF_TESTSUITE
   "Create out-of-tree targets for building polarphp performance benchmarks."
   FALSE)

option(POLAR_INCLUDE_TESTS "Create targets for building/running tests." TRUE)

option(POLAR_INCLUDE_DOCS
   "Create targets for building docs."
   TRUE)

set(POLAR_TARGET_ARCH "host"
   CACHE STRING "Set target to use for polarPHP JIT or use \"host\" for automatic detection.")

option(BUILD_SHARED_LIBS
   "Build all libraries as shared libraries instead of static" OFF)

#
# Miscellaneous User-configurable options.
#

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
   set(CMAKE_BUILD_TYPE "Debug" CACHE STRING
      "Build type for Swift [Debug, RelWithDebInfo, Release, MinSizeRel]"
      FORCE)
   message(STATUS "No build type was specified, will default to ${CMAKE_BUILD_TYPE}")
endif()

set(POLAR_STDLIB_BUILD_TYPE "${CMAKE_BUILD_TYPE}" CACHE STRING
   "Build type for the Swift standard library [Debug, RelWithDebInfo, Release, MinSizeRel]")
set_property(CACHE POLAR_STDLIB_BUILD_TYPE PROPERTY
   STRINGS "Debug" "RelWithDebInfo" "Release" "MinSizeRel")

polar_is_build_type_optimized("${POLAR_STDLIB_BUILD_TYPE}" polar_optimized)

if(POLAR_optimized)
   set(POLAR_STDLIB_ASSERTIONS_default FALSE)
else()
   set(POLAR_STDLIB_ASSERTIONS_default TRUE)
endif()

option(POLAR_STDLIB_ASSERTIONS
   "Enable internal checks for the Swift standard library (useful for debugging the library itself, does not affect checks required for safety)"
   "${POLAR_STDLIB_ASSERTIONS_default}")

# POLAR_VERSION is deliberately /not/ cached so that an existing build directory
# can be reused when a new version of Swift comes out (assuming the user hasn't
# manually set it as part of their own CMake configuration).

if(NOT DEFINED POLAR_VERSION_MAJOR)
   set(POLAR_VERSION_MAJOR 0)
endif()
if(NOT DEFINED POLAR_VERSION_MINOR)
   set(POLAR_VERSION_MINOR 0)
endif()
if(NOT DEFINED POLAR_VERSION_PATCH)
   set(POLAR_VERSION_PATCH 1)
endif()
if(NOT DEFINED POLAR_VERSION_SUFFIX)
   set(POLAR_VERSION_SUFFIX "")
endif()

set(POLAR_PACKAGE_VERSION "${POLAR_VERSION_MAJOR}.${POLAR_VERSION_MINOR}.${POLAR_VERSION_PATCH}")
set(POLAR_PACKAGE_NAME polarPHP)
set(POLAR_PACKAGE_STRING "${POLAR_PACKAGE_NAME} ${POLAR_PACKAGE_VERSION}")
set(POLAR_BUG_REPORT_URL "http://polarphp.org/bugs/")

set(POLAR_VENDOR "" CACHE STRING
   "The vendor name of the polarphp virtual machine")

# Indicate whether polarphp should attempt to use the lld linker.
set(POLAR_ENABLE_LLD_LINKER FALSE CACHE BOOL
   "Enable using the lld linker when available")

# Indicate whether polarphp should attempt to use the gold linker.
# This is not used on Darwin.
set(POLAR_ENABLE_GOLD_LINKER FALSE CACHE BOOL
   "Enable using the gold linker when available")

set(POLAR_TOOLS_ENABLE_LTO OFF CACHE STRING "Build PolarPHP tools with LTO. One
   must specify the form of LTO by setting this to one of: 'full', 'thin'. This
   option only affects the tools that run on the host (the compiler), and has
   no effect on the target libraries (the standard library and the runtime).")

# The following only works with the Ninja generator in CMake >= 3.0.
set(POLAR_PARALLEL_LINK_JOBS "" CACHE STRING
   "Define the maximum number of linker jobs for polarPHP.")

option(POLAR_FORCE_OPTIMIZED_TYPECHECKER "Override the optimization setting of
   the type checker so that it always compiles with optimization. This eases
   debugging after type checking occurs by speeding up type checking" FALSE)

#
# Assume a new enough ar to generate the index at construction time. This avoids
# having to invoke ranlib as a secondary command.
#
set(CMAKE_C_ARCHIVE_CREATE "<CMAKE_AR> crs <TARGET> <LINK_FLAGS> <OBJECTS>")
set(CMAKE_C_ARCHIVE_APPEND "<CMAKE_AR> qs <TARGET> <LINK_FLAGS> <OBJECTS>")
set(CMAKE_C_ARCHIVE_FINISH "")

set(CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> crs <TARGET> <LINK_FLAGS> <OBJECTS>")
set(CMAKE_CXX_ARCHIVE_APPEND "<CMAKE_AR> qs <TARGET> <LINK_FLAGS> <OBJECTS>")
set(CMAKE_CXX_ARCHIVE_FINISH "")

set(POLAR_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(POLAR_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")
set(POLAR_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")
set(POLAR_MAIN_INCLUDE_DIR "${POLAR_SOURCE_DIR}/include")
set(POLAR_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/include")

set(POLAR_RUNTIME_OUTPUT_INTDIR "${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/bin")
set(POLAR_LIBRARY_OUTPUT_INTDIR "${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/lib")
if("${POLAR_NATIVE_POLAR_TOOLS_PATH}" STREQUAL "")
   set(POLAR_NATIVE_POLAR_TOOLS_PATH "${POLAR_RUNTIME_OUTPUT_INTDIR}")
endif()

#
# User-configurable ICU specific options for Android, FreeBSD, Linux and Haiku.
#

foreach(_platform FREEBSD;LINUX;WINDOWS;HAIKU)
   set(POLAR_${_platform}_ICU_UC "" CACHE STRING
      "Path to a directory containing the icuuc library for ${_platform}")
   set(POLAR_${_platform}_ICU_UC_INCLUDE "" CACHE STRING
      "Path to a directory containing headers for icuuc for ${_platform}")
   set(POLAR_${_platform}_ICU_I18N "" CACHE STRING
      "Path to a directory containing the icui18n library for ${_platform}")
   set(POLAR_${_platform}_ICU_I18N_INCLUDE "" CACHE STRING
      "Path to a directory containing headers icui18n for ${_platform}")
endforeach()

if (BUILD_SHARED_LIBS)
   set(POLAR_SHARED ON)
else()
   set(POLAR_STATIC ON)
endif()

#
# User-configurable Darwin-specific options.
#
# None
#

#
# End of user-configurable options.
#

# All options referred to from HandlePolarOptions have to be specified
# BEFORE this include, otherwise options will not be correctly set on
# first cmake run
include(HandlePolarOptions)

include(LinkerProb)
include(Config-ix)

polar_precondition(CMAKE_SYSTEM_NAME)

#
# Include CMake modules
#
include(CheckCXXSourceRuns)
include(CMakeParseArguments)
include(CMakePushCheckState)

polar_print_versions()

include(PolarComponents)
include(PolarSetIfArchBitness)
include(PolarProcessSources)
include(AddPolar)
include(PolarList)
include(HandlePolarOptions)
include(AddPolarUnittests)

# Configure polarphp include, install, build components.
polar_configure_components()

# This setting causes all CMakeLists.txt to automatically have
# ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CURRENT_SOURCE_DIR} as an
# include_directories path. This is done for developer
# convenience.
#
# *NOTE* Even though these directories are added to the include path for a
# specific CMakeLists.txt, these include paths are not propagated down to
# subdirectories.
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# We'll need this once we have generated headers
include_directories(BEFORE
   ${POLAR_MAIN_INCLUDE_DIR}
   ${POLAR_INCLUDE_DIR}
   )

# Xcode: use libc++ and c++11 using proper build settings.
if(XCODE)
   polar_common_xcode_cxx_config()
endif()


# Enable additional warnings.

polar_common_cxx_warnings()

include(PolarCheckCXXNativeRegex)
polar_check_cxx_native_regex(POLAR_HAVE_WORKING_STD_REGEX)

if(POLAR_PARALLEL_LINK_JOBS)
   if(NOT CMAKE_MAKE_PROGRAM MATCHES "ninja")
      message(WARNING "Job pooling is only available with Ninja generators.")
   else()
      set_property(GLOBAL APPEND PROPERTY JOB_POOLS polar_link_job_pool=${POLAR_PARALLEL_LINK_JOBS})
      set(CMAKE_JOB_POOL_LINK polar_link_job_pool)
   endif()
endif()

#
# Find required dependencies.
#
if(NOT CMAKE_SYSTEM_NAME STREQUAL "Darwin" AND "${POLAR_PATH_TO_LIBICU_BUILD}" STREQUAL "")
   find_package(ICU REQUIRED COMPONENTS uc i18n)
endif()

#
# Find optional dependencies.
#

# Configure the three polarPHP configuration header files.
configure_file(
   ${POLAR_MAIN_INCLUDE_DIR}/polar/global/Config.h.cmake
   ${POLAR_INCLUDE_DIR}/polar/global/Config.h)
configure_file(
   ${POLAR_MAIN_INCLUDE_DIR}/polar/global/PolarConfig.h.cmake
   ${POLAR_INCLUDE_DIR}/polar/global/PolarConfig.h)

if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND POLAR_INCLUDE_TESTS)
   set(PPOLAR_FOUND_NATIVE_GTEST OFF)
   find_package(GTest)
   if(GTEST_FOUND)
      set(POLAR_FOUND_NATIVE_GTEST ON)
   endif()
   if(NOT POLAR_FOUND_NATIVE_GTEST)
      add_subdirectory(thirdparty/unittest)
   endif()
   add_subdirectory(unittests)
endif()
if(POLAR_INCLUDE_DOCS)
   add_subdirectory(docs)
endif()

add_subdirectory(cmake/modules)

#polar_install_in_component(license
#   FILES "LICENSE.txt"
#   DESTINATION "share/polarphp")

if(XCODE)
   add_custom_target(Documentation
      SOURCES
      README.md
      docs)
   file(GLOB POLAR_TOPLEVEL_HEADERS
      ${CMAKE_CURRENT_SOURCE_DIR}/include/polar${dir}/*.h
      ${CMAKE_CURRENT_SOURCE_DIR}/include/polar${dir}/*.def)
   
   add_custom_target(Miscellaneous
      SOURCES ${POLAR_TOPLEVEL_HEADERS})
endif()

add_subdirectory(src)

