polar_collect_files(
   TYPE_SOURCE
   RELATIVE
   DIR ${CMAKE_CURRENT_SOURCE_DIR}
   OUTPUT_VAR POLAR_BASIC_ADT_SOURCES)

add_library(PolarBasic STATIC ${POLAR_BASIC_ADT_SOURCES})
target_include_directories(PolarBasic PUBLIC ${POLAR_MAIN_INCLUDE_DIR}/polar/basic/adt)
