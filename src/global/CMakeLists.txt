polar_collect_files(
   TYPE_SOURCE
   RELATIVE
   DIR ${CMAKE_CURRENT_SOURCE_DIR}
   OUTPUT_VAR POLAR_GLOABL_SOURCES)

add_library(PolarGlobal OBJECT ${POLAR_GLOABL_SOURCES})
target_include_directories(PolarGlobal PUBLIC ${POLAR_MAIN_INCLUDE_DIR}/polar/global)
