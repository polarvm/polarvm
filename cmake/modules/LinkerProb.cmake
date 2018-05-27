if(NOT WIN32 AND NOT APPLE)
   # Detect what linker we have here
   if(POLAR_USE_LINKER)
      set(command ${CMAKE_C_COMPILER} -fuse-ld=${POLAR_USE_LINKER} -Wl,--version)
   else()
      set(command ${CMAKE_C_COMPILER} -Wl,--version)
   endif()
   execute_process(
      COMMAND ${command}
      OUTPUT_VARIABLE stdout
      ERROR_VARIABLE stderr
      )
   set(POLAR_LINKER_DETECTED ON)
   if("${stdout}" MATCHES "GNU gold")
      set(POLAR_LINKER_IS_GOLD ON)
      message(STATUS "Linker detection: GNU Gold")
   elseif("${stdout}" MATCHES "^LLD")
      set(POLAR_LINKER_IS_LLD ON)
      message(STATUS "Linker detection: LLD")
   elseif("${stdout}" MATCHES "GNU ld")
      set(POLAR_LINKER_IS_GNULD ON)
      message(STATUS "Linker detection: GNU ld")
   elseif("${stderr}" MATCHES "Solaris Link Editors" OR
         "${stdout}" MATCHES "Solaris Link Editors")
      set(POLAR_LINKER_IS_SOLARISLD ON)
      message(STATUS "Linker detection: Solaris ld")
   else()
      set(POLAR_LINKER_DETECTED OFF)
      message(STATUS "Linker detection: unknown")
   endif()
endif()
