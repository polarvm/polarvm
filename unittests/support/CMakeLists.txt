polar_collect_files(
   TYPE_BOTH
   RELATIVE
   DIR ${CMAKE_CURRENT_SOURCE_DIR}
   OUTPUT_VAR POLAR_TEST_SUPPORT_SOURCES)

add_library(TestSupport STATIC ${POLAR_TEST_SUPPORT_SOURCES})
target_link_libraries(TestSupport PolarUtils )
