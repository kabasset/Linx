CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

elements_subdir(RasterGeometry)

elements_depends_on_subdirs(LitlRaster)

find_package(Eigen3)

elements_add_library(RasterGeometry src/lib/*.cpp
                     INCLUDE_DIRS LitlRaster Eigen3
                     LINK_LIBRARIES LitlRaster Eigen3
                     PUBLIC_HEADERS RasterGeometry)

elements_add_unit_test(Interpolation tests/src/Interpolation_test.cpp 
                     EXECUTABLE RasterGeometry_Interpolation_test
                     LINK_LIBRARIES RasterGeometry
                     TYPE Boost)
elements_add_unit_test(Affinity tests/src/Affinity_test.cpp 
                     EXECUTABLE RasterGeometry_Affinity_test
                     LINK_LIBRARIES RasterGeometry
                     TYPE Boost)
