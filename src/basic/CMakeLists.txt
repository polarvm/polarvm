polar_collect_files(
   TYPE_SOURCE
   RELATIVE
   DIR ${CMAKE_CURRENT_SOURCE_DIR}
   OUTPUT_VAR POLAR_BASIC_ADT_SOURCES)

polar_add_library(PolarBasic
   ${POLAR_BASIC_ADT_SOURCES}
   ADDITIONAL_HEADER_DIRS
   ${POLAR_MAIN_INCLUDE_DIR}/polar/basic/adt
   LINK_LIBS ${system_libs}
   )
