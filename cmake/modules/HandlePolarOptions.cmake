# This CMake module is responsible for interpreting the user defined POLAR_
# options and executing the appropriate CMake commands to realize the users'
# selections.

# This is commonly needed so make sure it's defined before we include anything
# else.
include(CheckCompilerVersion)
include(HandlePolarStdlib)
include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)

string(TOUPPER "${CMAKE_BUILD_TYPE}" uppercase_CMAKE_BUILD_TYPE)

# Common cmake project config for additional warnings.
#
macro(polar_common_cxx_warnings)

endmacro()
