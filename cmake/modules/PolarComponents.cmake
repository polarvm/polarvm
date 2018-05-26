macro(polar_configure_components)
   
endmacro()

# Sets the is_installing variable.
function(polar_is_installing_component component result_var_name)
   polar_precondition(component MESSAGE "Component name is required")
   if("${component}" STREQUAL "never_install")
      set("${result_var_name}" FALSE PARENT_SCOPE)
   else()
      if(NOT "${component}" IN_LIST _POLAR_DEFINED_COMPONENTS)
         message(FATAL_ERROR "unknown install component: ${component}")
      endif()
      string(TOUPPER "${component}" var_name_piece)
      string(REPLACE "-" "_" var_name_piece "${var_name_piece}")
      set("${result_var_name}" "${POLAR_INSTALL_${var_name_piece}}" PARENT_SCOPE)
   endif()
endfunction()

# polar_install_in_component(<COMPONENT NAME>
#   <same parameters as install()>)
#
# Executes the specified installation actions if the named component is
# requested to be installed.
#
# This function accepts the same parameters as install().
function(polar_install_in_component component)
   polar_precondition(component MESSAGE "Component name is required")
   polar_is_installing_component("${component}" is_installing)
   if(is_installing)
      install(${ARGN})
   endif()
endfunction()
