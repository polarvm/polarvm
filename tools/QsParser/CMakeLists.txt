add_subdirectory(syntax)
add_subdirectory(lib)

polar_add_bootstrap_tool(QsParser
   main.cpp
   )

target_link_libraries(QsParser PRIVATE qsparserlib)

