#file(GLOB_RECURSE QS_SCRIPT_FILES
#   LIST_DIRECTORIES false
#   RELATIVE ${POLAR_SOURCE_DIR}
#   "${POLAR_MAIN_INCLUDE_DIR}/*.qs")

#file(GLOB_RECURSE SRC_QS_SCRIPT_FILES
#   LIST_DIRECTORIES false
#   RELATIVE ${POLAR_SOURCE_DIR}
#   "${POLAR_LIB_SOURCE_DIR}/*.qs")

#foreach (item ${SRC_QS_SCRIPT_FILES})
#   list(APPEND QS_SCRIPT_FILES ${item})
#endforeach()

#set(QS_SCRIPT_GENERATED_FILES)
#set(QS_SCRIPT_REAL_FILES)

#set(QS_GENERATED_DIR ${POLAR_BINARY_DIR}/qsgeneratedfiles)

#foreach (item ${QS_SCRIPT_FILES})
#   string(REPLACE ".qs" "" generated_source_filename ${item})
#   list(APPEND QS_SCRIPT_GENERATED_FILES ${QS_GENERATED_DIR}/${generated_source_filename})
#   list(APPEND QS_SCRIPT_REAL_FILES ${POLAR_SOURCE_DIR}/${item})
#endforeach()

#if (NOT EXISTS ${QS_GENERATED_DIR})
#   file(MAKE_DIRECTORY ${QS_GENERATED_DIR})
#endif()

#set(QS_GENERATOR_FILENAME ${QS_GENERATED_DIR}/qsgenerator.cpp)

#add_custom_command(OUTPUT ${QS_GENERATOR_FILENAME}
#   COMMAND touch ${QS_GENERATOR_FILENAME}
#   COMMAND echo
#   ARGS
#   \"int main(){}\"
#   >>
#    ${QS_GENERATOR_FILENAME}
#   DEPENDS ${QS_SCRIPT_REAL_FILES})

#add_executable(qsgenerator ${QS_GENERATOR_FILENAME})

#add_custom_command(OUTPUT ${QS_SCRIPT_GENERATED_FILES}
#   COMMAND qsgenerator
#   DEPENDS qsgenerator)

#add_library(xxxxx ${QS_SCRIPT_GENERATED_FILES})
