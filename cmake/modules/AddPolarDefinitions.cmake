# There is no clear way of keeping track of compiler command-line
# options chosen via `add_definitions'

# Beware that there is no implementation of remove_llvm_definitions.

macro(polar_add_llvm_definitions)
   # We don't want no semicolons on POLAR_DEFINITIONS:
   foreach(arg ${ARGN})
      if(DEFINED POLAR_DEFINITIONS)
         set(POLAR_DEFINITIONS "${POLAR_DEFINITIONS} ${arg}")
      else()
         set(POLAR_DEFINITIONS ${arg})
      endif()
   endforeach(arg)
   add_definitions( ${ARGN} )
endmacro(polar_add_llvm_definitions)
