CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

elements_subdir(RasterFilter)

elements_depends_on_subdirs(Raster RasterGeometry)

elements_add_library(RasterFilter src/lib/*.cpp
                     INCLUDE_DIRS Raster RasterGeometry
                     LINK_LIBRARIES Raster RasterGeometry
                     PUBLIC_HEADERS RasterFilter)

elements_add_unit_test(Convolution tests/src/Convolution_test.cpp 
                     EXECUTABLE RasterFilter_Convolution_test
                     LINK_LIBRARIES RasterFilter
                     TYPE Boost)
elements_add_unit_test(Kernel tests/src/Kernel_test.cpp 
                     EXECUTABLE RasterFilter_Kernel_test
                     LINK_LIBRARIES RasterFilter
                     TYPE Boost)
elements_add_unit_test(Kernel1d tests/src/Kernel1d_test.cpp 
                     EXECUTABLE RasterFilter_Kernel1d_test
                     LINK_LIBRARIES RasterFilter
                     TYPE Boost)
