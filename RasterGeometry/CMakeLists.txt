CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

elements_subdir(RasterGeometry)

elements_depends_on_subdirs(Raster)


elements_add_library(RasterGeometry src/lib/*.cpp
                     INCLUDE_DIRS Raster
                     LINK_LIBRARIES Raster
                     PUBLIC_HEADERS RasterGeometry)

elements_add_unit_test(Interpolation tests/src/Interpolation_test.cpp 
                     EXECUTABLE RasterGeometry_Interpolation_test
                     LINK_LIBRARIES RasterGeometry
                     TYPE Boost)
elements_add_unit_test(Extrapolation tests/src/Extrapolation_test.cpp 
                     EXECUTABLE RasterGeometry_Extrapolation_test
                     LINK_LIBRARIES RasterGeometry
                     TYPE Boost)
elements_add_unit_test(Affinity tests/src/Affinity_test.cpp 
                     EXECUTABLE RasterGeometry_Affinity_test
                     LINK_LIBRARIES RasterGeometry
                     TYPE Boost)
