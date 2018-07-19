polar_add_unittest(PolarToolTests QsParserTest
   EngineTest.cpp
   ../../TestEntry.cpp
   )

target_link_libraries(QsParserTest PRIVATE TestSupport qsparserlib)
include_directories(${POLAR_TOOLS_DIR})
set(DEFINES
   QSPARSER_OUTPUT_DIR="${CMAKE_CURRENT_BINARY_DIR}/outputDir"
   QSPARSER_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")
set_target_properties(QsParserTest PROPERTIES
   COMPILE_DEFINITIONS "${DEFINES}")
