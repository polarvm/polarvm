add_custom_target(PolarBaseLibTests)
set_target_properties(PolarBaseLibTests PROPERTIES FOLDER "PolarBaseLibTests")

add_custom_target(PolarToolTests)
set_target_properties(PolarToolTests PROPERTIES FOLDER "PolarToolTests")

add_subdirectory(support)
add_subdirectory(basic/adt)
add_subdirectory(utils)
add_subdirectory(tools)
