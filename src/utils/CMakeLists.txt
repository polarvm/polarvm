set(system_libs)

if(MSVC OR MINGW)
   # libuuid required for FOLDERID_profile usage in lib/Support/Windows/Path.inc.
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
   list(APPEND POLAR_UTILS_SOURCES _platform/DynamicLibraryUnix.cpp)
   list(APPEND POLAR_UTILS_SOURCES _platform/WatchDogUnix.cpp)
endif()

#message("${POLAR_UTILS_SOURCES}")

set(POLAR_UTILS_SOURCES
   ARMAttributeParser.cpp;
   ARMBuildAttrs.cpp;
   ARMWinEH.cpp;
   Allocator.cpp;
   BinaryStreamError.cpp;
   BinaryStreamReader.cpp;
   BinaryStreamRef.cpp;
   BinaryStreamWriter.cpp;
#   BlockFrequency.cpp;
#   BranchProbability.cpp;
#   CachePruning.cpp;
#   Chrono.cpp;
   CircleRawOutStream.cpp;
#   CommandLine.cpp;
#   Compression.cpp;
   ConvertUtf.cpp;
   ConvertUtfWrapper.cpp;
#   CrashRecoveryContext.cpp;
#   DataExtractor.cpp;
#   Debug.cpp;
#   DebugCounter.cpp;
#   DynamicLibrary.cpp;
   ErrorHandling.cpp;
   ErrorNumber.cpp;
   ErrorType.cpp;
   FileOutputBuffer.cpp;
   FileUtils.cpp;
#   FormatVariadic.cpp;
#   FormattedStream.cpp;
#   GlobPattern.cpp;
#   GraphWriter.cpp;
#   Host.cpp;
#   JamCrc.cpp;
#   KnownBits.cpp;
#   Leb128.cpp;
#   LineIterator.cpp;
#   Locale.cpp;
   LockFileMgr.cpp;
   MathExtras.cpp;
   Md5.cpp;
   MemoryBuffer.cpp;
   NativeFormatting.cpp;
#   Options.cpp;
#   Parallel.cpp;
#   Path.cpp;
#   PluginLoader.cpp;
#   PrettyStackTrace.cpp;
#   Process.cpp;
#   Program.cpp;
   RandomNumberGenerator.cpp;
#   RawOsOutStream.cpp;
#   RawOutStream.cpp;
   ScaledNumber.cpp;
   ScopedPrinter.cpp;
   Sha1.cpp;
#   Signals.cpp;
   SourceMgr.cpp;
   SpecialCaseList.cpp;
#   StringPool.cpp;
#   StringSaver.cpp;
   SystemUtils.cpp;
   TargetParser.cpp;
#   Timer.cpp;
#   ToolOutputFile.cpp;
   Unicode.cpp;
#   Valgrind.cpp;
#   yaml/YamlParser.cpp;
#   yaml/YamlTraits.cpp;
#   _platform/ProgramUnix.cpp;
#   _platform/ProcessUnix.cpp;
#   _platform/PathUnix.cpp;
#   _platform/SignalsUnix.cpp;
#   _platform/HostUnix.cpp;
   _platform/MemoryUnix.cpp;
#   _platform/DynamicLibraryUnix.cpp;
   _platform/WatchDogUnix.cpp
   )

polar_add_library(PolarUtils
   ${POLAR_UTILS_SOURCES}
   ADDITIONAL_HEADER_DIRS
   Unix
   Windows
   ${POLAR_MAIN_INCLUDE_DIR}/polar/utils
   LINK_LIBS ${system_libs} PolarGlobal PolarBasic
   )
