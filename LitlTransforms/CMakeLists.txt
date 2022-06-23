CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

elements_subdir(LitlTransforms)

elements_depends_on_subdirs(LitlRaster)

find_package(Eigen3)

elements_add_library(LitlTransforms src/lib/*.cpp
                     INCLUDE_DIRS LitlRaster Eigen3
                     LINK_LIBRARIES LitlRaster Eigen3
                     PUBLIC_HEADERS LitlTransforms)

elements_add_unit_test(Interpolation tests/src/Interpolation_test.cpp 
                     EXECUTABLE LitlTransforms_Interpolation_test
                     LINK_LIBRARIES LitlTransforms
                     TYPE Boost)
elements_add_unit_test(Affinity tests/src/Affinity_test.cpp 
                     EXECUTABLE LitlTransforms_Affinity_test
                     LINK_LIBRARIES LitlTransforms
                     TYPE Boost)
