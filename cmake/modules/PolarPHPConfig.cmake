# polar_is_polarphp_target_library(
#   library
#     Name of the POLAR library to check
#   return_var
#     Output variable name
#   ALL_TARGETS;INCLUDED_TARGETS;OMITTED_TARGETS
#     ALL_TARGETS - default looks at the full list of known targets
#     INCLUDED_TARGETS - looks only at targets being configured
#     OMITTED_TARGETS - looks only at targets that are not being configured
# )
function(polar_is_polarphp_target_library library return_var)
#   cmake_parse_arguments(ARG "ALL_TARGETS;INCLUDED_TARGETS;OMITTED_TARGETS" "" "" ${ARGN})
#   # Sets variable `return_var' to ON if `library' corresponds to a
#   # POLAR supported target. To OFF if it doesn't.
#   set(${return_var} OFF PARENT_SCOPE)
#   string(TOUPPER "${library}" capitalized_lib)
#   if(ARG_INCLUDED_TARGETS)
#      string(TOUPPER "${POLAR_TARGETS_TO_BUILD}" targets)
#   elseif(ARG_OMITTED_TARGETS)
#      set(omitted_targets ${POLAR_ALL_TARGETS})
#      list(REMOVE_ITEM omitted_targets ${POLAR_TARGETS_TO_BUILD})
#      string(TOUPPER "${omitted_targets}" targets)
#   else()
#      string(TOUPPER "${POLAR_ALL_TARGETS}" targets)
#   endif()
#   foreach(t ${targets})
#      if( capitalized_lib STREQUAL t OR
#            capitalized_lib STREQUAL "${t}" OR
#            capitalized_lib STREQUAL "${t}UTILS" )
#         set(${return_var} ON PARENT_SCOPE)
#         break()
#      endif()
#   endforeach()
endfunction(polar_is_polarphp_target_library)

function(polar_is_polarphp_target_specifier library return_var)
   polar_is_polarphp_target_library(${library} ${return_var} ${ARGN})
   string(TOUPPER "${library}" capitalized_lib)
   if(NOT ${return_var})
      if( capitalized_lib STREQUAL "ALLTARGETSASMPARSERS" OR
            capitalized_lib STREQUAL "ALLTARGETSDESCS" OR
            capitalized_lib STREQUAL "ALLTARGETSDISASSEMBLERS" OR
            capitalized_lib STREQUAL "ALLTARGETSINFOS" OR
            capitalized_lib STREQUAL "NATIVE" OR
            capitalized_lib STREQUAL "NATIVECODEGEN" )
         set(${return_var} ON PARENT_SCOPE)
      endif()
   endif()
endfunction()

macro(polar_config executable)
   cmake_parse_arguments(ARG "USE_SHARED" "" "" ${ARGN})
   set(link_components ${ARG_UNPARSED_ARGUMENTS})

   if(USE_SHARED)
      # If USE_SHARED is specified, then we link against libPolarPHP,
      # but also against the component libraries below. This is
      # done in case libPOLAR does not contain all of the components
      # the target requires.
      #
      # Strip POLAR_DYLIB_COMPONENTS out of link_components.
      # To do this, we need special handling for "all", since that
      # may imply linking to libraries that are not included in
      # libPolarPHP.

      if (DEFINED link_components AND DEFINED POLAR_DYLIB_COMPONENTS)
         if("${POLAR_DYLIB_COMPONENTS}" STREQUAL "all")
            set(link_components "")
         else()
            list(REMOVE_ITEM link_components ${POLAR_DYLIB_COMPONENTS})
         endif()
      endif()

      target_link_libraries(${executable} PRIVATE PolarPHP)
   endif()

   polar_explicit_config(${executable} ${link_components})
endmacro(polar_config)

function(polar_explicit_config executable)
   set( link_components ${ARGN} )

   polar_map_components_to_libnames(LIBRARIES ${link_components})
   get_target_property(t ${executable} TYPE)
   if(t STREQUAL "STATIC_LIBRARY")
      target_link_libraries(${executable} INTERFACE ${LIBRARIES})
   elseif(t STREQUAL "EXECUTABLE" OR t STREQUAL "SHARED_LIBRARY" OR t STREQUAL "MODULE_LIBRARY")
      target_link_libraries(${executable} PRIVATE ${LIBRARIES})
   else()
      # Use plain form for legacy user.
      target_link_libraries(${executable} ${LIBRARIES})
   endif()
endfunction(polar_explicit_config)

# This is a variant intended for the final user:
# Map LINK_COMPONENTS to actual libnames.
function(polar_map_components_to_libnames out_libs)
   set( link_components ${ARGN} )
   if(NOT POLAR_AVAILABLE_LIBS)
      # Inside POLAR itself available libs are in a global property.
      get_property(POLAR_AVAILABLE_LIBS GLOBAL PROPERTY POLAR_LIBS)
   endif()
   string(TOUPPER "${POLAR_AVAILABLE_LIBS}" capitalized_libs)

   get_property(POLAR_TARGETS_CONFIGURED GLOBAL PROPERTY POLAR_TARGETS_CONFIGURED)

   # Generally in our build system we avoid order-dependence. Unfortunately since
   # not all targets create the same set of libraries we actually need to ensure
   # that all build targets associated with a target are added before we can
   # process target dependencies.
   if(NOT POLAR_TARGETS_CONFIGURED)
      foreach(c ${link_components})
         polar_is_polarphp_target_specifier(${c} iltl_result ALL_TARGETS)
         if(iltl_result)
            message(FATAL_ERROR "Specified target library before target registration is complete.")
         endif()
      endforeach()
   endif()

   # Expand some keywords:
   list(FIND POLAR_TARGETS_TO_BUILD "${POLAR_NATIVE_ARCH}" have_native_backend)

   # Translate symbolic component names to real libraries:
   foreach(c ${link_components})
      # add codegen, asmprinter, asmparser, disassembler
      list(FIND POLAR_TARGETS_TO_BUILD ${c} idx)
      if( NOT idx LESS 0 )
         if( TARGET POLAR${c}CodeGen )
            list(APPEND expanded_components "Polar${c}CodeGen")
         else()
            if( TARGET POLAR${c} )
               list(APPEND expanded_components "Polar${c}")
            else()
               message(FATAL_ERROR "Target ${c} is not in the set of libraries.")
            endif()
         endif()
         if( TARGET Polar${c}AsmParser )
            list(APPEND expanded_components "Polar${c}AsmParser")
         endif()
         if( TARGET Polar${c}AsmPrinter )
            list(APPEND expanded_components "Polar${c}AsmPrinter")
         endif()
         if( TARGET Polar${c}Desc )
            list(APPEND expanded_components "Polar${c}Desc")
         endif()
         if( TARGET Polar${c}Disassembler )
            list(APPEND expanded_components "Polar${c}Disassembler")
         endif()
         if( TARGET Polar${c}Info )
            list(APPEND expanded_components "Polar${c}Info")
         endif()
         if( TARGET Polar${c}Utils )
            list(APPEND expanded_components "Polar${c}Utils")
         endif()
      elseif( c STREQUAL "native" )
         # already processed
      elseif( c STREQUAL "nativecodegen" )
         list(APPEND expanded_components "POLAR${POLAR_NATIVE_ARCH}CodeGen")
         if( TARGET POLAR${POLAR_NATIVE_ARCH}Desc )
            list(APPEND expanded_components "POLAR${POLAR_NATIVE_ARCH}Desc")
         endif()
         if( TARGET POLAR${POLAR_NATIVE_ARCH}Info )
            list(APPEND expanded_components "Polar${POLAR_NATIVE_ARCH}Info")
         endif()
      elseif( c STREQUAL "backend" )
         # same case as in `native'.
      elseif( c STREQUAL "engine" )
         # already processed
      elseif( c STREQUAL "all" )
         list(APPEND expanded_components ${POLAR_AVAILABLE_LIBS})
      elseif( c STREQUAL "AllTargetsAsmPrinters" )
         # Link all the asm printers from all the targets
         foreach(t ${POLAR_TARGETS_TO_BUILD})
            if( TARGET Polar${t}AsmPrinter )
               list(APPEND expanded_components "Polar${t}AsmPrinter")
            endif()
         endforeach(t)
      elseif( c STREQUAL "AllTargetsAsmParsers" )
         # Link all the asm parsers from all the targets
         foreach(t ${POLAR_TARGETS_TO_BUILD})
            if( TARGET Polar${t}AsmParser )
               list(APPEND expanded_components "Polar${t}AsmParser")
            endif()
         endforeach(t)
      elseif( c STREQUAL "AllTargetsDescs" )
         # Link all the descs from all the targets
         foreach(t ${POLAR_TARGETS_TO_BUILD})
            if( TARGET Polar${t}Desc )
               list(APPEND expanded_components "Polar${t}Desc")
            endif()
         endforeach(t)
      elseif( c STREQUAL "AllTargetsDisassemblers" )
         # Link all the disassemblers from all the targets
         foreach(t ${POLAR_TARGETS_TO_BUILD})
            if( TARGET Polar${t}Disassembler )
               list(APPEND expanded_components "Polar${t}Disassembler")
            endif()
         endforeach(t)
      elseif( c STREQUAL "AllTargetsInfos" )
         # Link all the infos from all the targets
         foreach(t ${POLAR_TARGETS_TO_BUILD})
            if( TARGET Polar${t}Info )
               list(APPEND expanded_components "Polar${t}Info")
            endif()
         endforeach(t)
      else( NOT idx LESS 0 )
         # Canonize the component name:
         string(TOUPPER "${c}" capitalized)
         list(FIND capitalized_libs Polar${capitalized} lib_idx)
         if( lib_idx LESS 0 )
            # The component is unknown. Maybe is an omitted target?
            polar_is_polarphp_target_library(${c} iltl_result OMITTED_TARGETS)
            if(iltl_result)
               # A missing library to a directly referenced omitted target would be bad.
               message(FATAL_ERROR "Library '${c}' is a direct reference to a target library for an omitted target.")
            else()
               # If it is not an omitted target we should assume it is a component
               # that hasn't yet been processed by CMake. Missing components will
               # cause errors later in the configuration, so we can safely assume
               # that this is valid here.
               list(APPEND expanded_components Polar${c})
            endif()
         else( lib_idx LESS 0 )
            list(GET POLAR_AVAILABLE_LIBS ${lib_idx} canonical_lib)
            list(APPEND expanded_components ${canonical_lib})
         endif( lib_idx LESS 0 )
      endif( NOT idx LESS 0 )
   endforeach(c)

   set(${out_libs} ${expanded_components} PARENT_SCOPE)
endfunction()

# Perform a post-order traversal of the dependency graph.
# This duplicates the algorithm used by polar-config, originally
# in tools/polar-config/polar-config.cpp, function ComputeLibsForComponents.
function(polar_expand_topologically name required_libs visited_libs)
   list(FIND visited_libs ${name} found)
   if( found LESS 0 )
      list(APPEND visited_libs ${name})
      set(visited_libs ${visited_libs} PARENT_SCOPE)

      get_property(lib_deps GLOBAL PROPERTY POLARBUILD_LIB_DEPS_${name})
      foreach( lib_dep ${lib_deps} )
         polar_expand_topologically(${lib_dep} "${required_libs}" "${visited_libs}")
         set(required_libs ${required_libs} PARENT_SCOPE)
         set(visited_libs ${visited_libs} PARENT_SCOPE)
      endforeach()

      list(APPEND required_libs ${name})
      set(required_libs ${required_libs} PARENT_SCOPE)
   endif()
endfunction()

# Expand dependencies while topologically sorting the list of libraries:
function(polar_expand_dependencies out_libs)
   set(expanded_components ${ARGN})

   set(required_libs)
   set(visited_libs)
   foreach( lib ${expanded_components} )
      polar_expand_topologically(${lib} "${required_libs}" "${visited_libs}")
   endforeach()

   list(REVERSE required_libs)
   set(${out_libs} ${required_libs} PARENT_SCOPE)
endfunction()

function(polar_explicit_map_components_to_libraries out_libs)
   polar_map_components_to_libnames(link_libs ${ARGN})
   polar_expand_dependencies(expanded_components ${link_libs})
   # Return just the libraries included in this build:
   set(result)
   foreach(c ${expanded_components})
      if( TARGET ${c} )
         set(result ${result} ${c})
      endif()
   endforeach(c)
   set(${out_libs} ${result} PARENT_SCOPE)
endfunction(polar_explicit_map_components_to_libraries)
