add_custom_target(PolarPHPUnitTests)
set_target_properties(PolarPHPUnitTests PROPERTIES FOLDER "Tests")

# Generic support for adding a unittest.
function(polar_add_unittest test_suite test_name)
   if(NOT PDK_BUILD_TESTS)
      set(EXCLUDE_FROM_ALL ON)
   endif()
   
endfunction()
