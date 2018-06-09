# This CMake module is responsible for setting the standard library to libc++
# if the user has requested it.

include(DetermineGCCCompatible)

if(NOT DEFINED POLAR_STDLIB_HANDLED)
   set(POLAR_STDLIB_HANDLED ON)
   include(CheckCXXCompilerFlag)
   if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
      check_cxx_compiler_flag("-stdlib=libc++" CXX_SUPPORTS_STDLIB)
      if(CXX_SUPPORTS_STDLIB)
         polar_append_flag("-stdlib=libc++"
            CMAKE_CXX_FLAGS CMAKE_EXE_LINKER_FLAGS CMAKE_SHARED_LINKER_FLAGS
            CMAKE_MODULE_LINKER_FLAGS)
      else()
         message(WARNING "Can't specify libc++ with '-stdlib='")
      endif()
   endif()
endif()
