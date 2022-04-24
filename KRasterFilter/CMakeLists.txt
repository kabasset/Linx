CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

#===============================================================================
# Load elements_subdir macro here
# Examples:
#   For declaring a project module:
#         elements_subdir(ElementsExamples)
#===============================================================================
elements_subdir(KRasterFilter)

#===============================================================================
# Load elements_depends_on_subdirs macro here
#   For creating a dependency onto an other accessible module
#         elements_depends_on_subdirs(ElementsKernel)
#===============================================================================
elements_depends_on_subdirs(KRaster)

#===============================================================================
# Add the find_package macro (a pure CMake command) here to locate the
# libraries.
# Examples:
#          find_package(CppUnit)
#===============================================================================

#===============================================================================
# Declare the library dependencies here
# Example:
#         elements_add_library(ElementsExamples src/Lib/*.cpp
#                     INCLUDE_DIRS Boost ElementsKernel
#                     LINK_LIBRARIES Boost ElementsKernel
#                     PUBLIC_HEADERS ElementsExamples)
#===============================================================================
elements_add_library(KRasterFilter src/lib/*.cpp
                     INCLUDE_DIRS KRaster
                     LINK_LIBRARIES KRaster
                     PUBLIC_HEADERS KRasterFilter)

#===============================================================================
# Declare the executables here
# Example:
# elements_add_executable(ElementsProgramExample src/Program/ProgramExample.cpp
#                        INCLUDE_DIRS Boost ElementsExamples
#                        LINK_LIBRARIES Boost ElementsExamples)
#===============================================================================

#===============================================================================
# Declare the Boost tests here
# Example:
# elements_add_unit_test(BoostClassExample tests/src/Boost/ClassExample_test.cpp
#                       EXECUTABLE BoostClassExample_test
#                       INCLUDE_DIRS ElementsExamples
#                       LINK_LIBRARIES ElementsExamples TYPE Boost)
#===============================================================================
elements_add_unit_test(Convolution tests/src/Convolution_test.cpp 
                     EXECUTABLE KRasterFilter_Convolution_test
                     LINK_LIBRARIES KRasterFilter
                     TYPE Boost)
elements_add_unit_test(Kernel tests/src/Kernel_test.cpp 
                     EXECUTABLE KRasterFilter_Kernel_test
                     LINK_LIBRARIES KRasterFilter
                     TYPE Boost)
elements_add_unit_test(Kernel1d tests/src/Kernel1d_test.cpp 
                     EXECUTABLE KRasterFilter_Kernel1d_test
                     LINK_LIBRARIES KRasterFilter
                     TYPE Boost)

#===============================================================================
# Use the following macro for python modules, scripts and aux files:
#  elements_install_python_modules()
#  elements_install_scripts()
#===============================================================================

#===============================================================================
# Declare the Python programs here
# Examples :
# elements_add_python_program(PythonProgramExample
#                             ElementsExamples.PythonProgramExample)
#===============================================================================

#===============================================================================
# Add the elements_install_conf_files macro
# Examples:
#          elements_install_conf_files()
#===============================================================================
