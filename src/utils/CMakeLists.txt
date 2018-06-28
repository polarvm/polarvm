set(system_libs)

if(MSVC OR MINGW)
   # libuuid required for FOLDERID_Profile usage in lib/Support/Windows/Path.inc.
   # advapi32 required for CryptAcquireContextW in lib/Support/Windows/Path.inc.
   set(system_libs ${system_libs} psapi shell32 ole32 uuid advapi32)
elseif(CMAKE_HOST_UNIX)
   if(HAVE_LIBRT)
      set(system_libs ${system_libs} rt)
   endif()
   if(HAVE_LIBDL)
      set(system_libs ${system_libs} ${CMAKE_DL_LIBS})
   endif()
   if(HAVE_BACKTRACE)
      set(system_libs ${system_libs} ${Backtrace_LIBRARIES})
   endif()
   if(POLAR_ENABLE_TERMINFO)
      if(HAVE_TERMINFO)
         set(system_libs ${system_libs} ${TERMINFO_LIBS})
      endif()
   endif()
   set(system_libs ${system_libs} ${POLAR_PTHREAD_LIB})
   if( UNIX AND NOT (BEOS OR HAIKU) )
      set(system_libs ${system_libs} m)
   endif()
endif(MSVC OR MINGW)

polar_collect_files(
   TYPE_SOURCE
   RELATIVE
   DIR ${CMAKE_CURRENT_SOURCE_DIR}
   OUTPUT_VAR POLAR_UTILS_SOURCES)

if(UNIX)
   list(APPEND POLAR_UTILS_SOURCES _platform/ProgramUnix.cpp)
   list(APPEND POLAR_UTILS_SOURCES _platform/ProcessUnix.cpp)
   list(APPEND POLAR_UTILS_SOURCES _platform/PathUnix.cpp)
   list(APPEND POLAR_UTILS_SOURCES _platform/SignalsUnix.cpp)
   list(APPEND POLAR_UTILS_SOURCES _platform/HostUnix.cpp)
   list(APPEND POLAR_UTILS_SOURCES _platform/MemoryUnix.cpp)
endif()

polar_add_library(PolarUtils
   ${POLAR_UTILS_SOURCES} $<TARGET_OBJECTS:PolarGlobal> $<TARGET_OBJECTS:PolarBasic>
   ADDITIONAL_HEADER_DIRS
   Unix
   Windows
   ${POLAR_MAIN_INCLUDE_DIR}/polar/utils
   LINK_LIBS ${system_libs}
   )
