include(CMakeParseArguments)

function(polar_precondition var)
   cmake_parse_arguments(
      PRECONDITION # prefix
      "NEGATE" # options
      "MESSAGE" # single-value args
      "" # multi-value args
      ${ARGN})
   
   if (PRECONDITION_NEGATE)
      if (${var})
         if (PRECONDITION_MESSAGE)
            message(FATAL_ERROR "Error! ${PRECONDITION_MESSAGE}")
         else()
            message(FATAL_ERROR "Error! Variable ${var} is true or not empty. The value of ${var} is ${${var}}.")
         endif()
      endif()
   else()
      if (NOT ${var})
         if (PRECONDITION_MESSAGE)
            message(FATAL_ERROR "Error! ${PRECONDITION_MESSAGE}")
         else()
            message(FATAL_ERROR "Error! Variable ${var} is false, empty or not set.")
         endif()
      endif()
   endif()
endfunction()

function(polar_is_build_type_optimized build_type result_var_name)
   if("${build_type}" STREQUAL "Debug")
      set("${result_var_name}" FALSE PARENT_SCOPE)
   elseif("${build_type}" STREQUAL "RelWithDebInfo" OR
         "${build_type}" STREQUAL "Release" OR
         "${build_type}" STREQUAL "MinSizeRel")
      set("${result_var_name}" TRUE PARENT_SCOPE)
   else()
      message(FATAL_ERROR "Unknown build type: ${build_type}")
   endif()
endfunction()

# Look for either a program in execute_process()'s path or for a hardcoded path.
# Find a program's version and set it in the parent scope.
# Replace newlines with spaces so it prints on one line.
function(polar_find_version cmd flag find_in_path)
   if(find_in_path)
      message(STATUS "Finding installed version for: ${cmd}")
   else()
      message(STATUS "Finding version for: ${cmd}")
   endif()
   execute_process(
      COMMAND ${cmd} ${flag}
      OUTPUT_VARIABLE out
      OUTPUT_STRIP_TRAILING_WHITESPACE)
   if(NOT out)
      if(find_in_path)
         message(STATUS "tried to find version for ${cmd}, but ${cmd} not found in path, continuing")
      else()
         message(FATAL_ERROR "tried to find version for ${cmd}, but ${cmd} not found")
      endif()
   else()
      string(REPLACE "\n" " " out2 ${out})
      message(STATUS "Found version: ${out2}")
   endif()
   message(STATUS "")
endfunction()

# Print out path and version of any installed commands.
# We migth be using the wrong version of a command, so record them all.
function(polar_print_versions)
   polar_find_version("cmake" "--version" TRUE)
   
   message(STATUS "Finding version for: ${CMAKE_COMMAND}")
   message(STATUS "Found version: ${CMAKE_VERSION}")
   message(STATUS "")
   
   get_filename_component(CMAKE_MAKE_PROGRAM_BN "${CMAKE_MAKE_PROGRAM}" NAME_WE)
   if(${CMAKE_MAKE_PROGRAM_BN} STREQUAL "ninja" OR
         ${CMAKE_MAKE_PROGRAM_BN} STREQUAL "make")
      polar_find_version(${CMAKE_MAKE_PROGRAM_BN} "--version" TRUE)
      polar_find_version(${CMAKE_MAKE_PROGRAM} "--version" FALSE)
   endif()
   
   if(${SWIFT_PATH_TO_CMARK_BUILD})
      polar_find_version("cmark" "--version" TRUE)
      polar_find_version("${SWIFT_PATH_TO_CMARK_BUILD}/src/cmark" "--version" FALSE)
   endif()
   
   message(STATUS "Finding version for: ${CMAKE_C_COMPILER}")
   message(STATUS "Found version: ${CMAKE_C_COMPILER_VERSION}")
   message(STATUS "")
   
   message(STATUS "Finding version for: ${CMAKE_CXX_COMPILER}")
   message(STATUS "Found version: ${CMAKE_CXX_COMPILER_VERSION}")
   message(STATUS "")
endfunction()

function(polar_append_value value)
   foreach(variable ${ARGN})
      set(${variable} "${${variable}} ${value}" PARENT_SCOPE)
   endforeach(variable)
endfunction()

function(polar_append_if condition value)
   if (${condition})
      foreach(variable ${ARGN})
         set(${variable} "${${variable}} ${value}" PARENT_SCOPE)
      endforeach(variable)
   endif()
endfunction()
