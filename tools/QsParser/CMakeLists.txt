polar_collect_files(
   TYPE_BOTH
   RELATIVE
   DIR ${CMAKE_CURRENT_SOURCE_DIR}
   OUTPUT_VAR QS_PARSER_SOURCES)

polar_add_bootstrap_tool(QsParser
   ${QS_PARSER_SOURCES})
