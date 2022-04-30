CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

elements_subdir(Raster)

elements_depends_on_subdirs(RasterTypes)
elements_depends_on_subdirs(ElementsKernel)

find_package(Boost)
elements_add_library(Raster src/lib/*.cpp
                     INCLUDE_DIRS RasterTypes Boost ElementsKernel
                     LINK_LIBRARIES RasterTypes Boost ElementsKernel
                     PUBLIC_HEADERS Raster)
elements_add_unit_test(ContiguousContainer tests/src/ContiguousContainer_test.cpp 
                     EXECUTABLE Raster_ContiguousContainer_test
                     LINK_LIBRARIES Raster
                     TYPE Boost)
elements_add_unit_test(DataContainer tests/src/DataContainer_test.cpp 
                     EXECUTABLE Raster_DataContainer_test
                     LINK_LIBRARIES Raster
                     TYPE Boost)
elements_add_unit_test(DataUtils tests/src/DataUtils_test.cpp 
                     EXECUTABLE Raster_DataUtils_test
                     LINK_LIBRARIES Raster
                     TYPE Boost)
elements_add_unit_test(Position tests/src/Position_test.cpp 
                     EXECUTABLE Raster_Position_test
                     LINK_LIBRARIES Raster
                     TYPE Boost)
elements_add_unit_test(PositionIterator tests/src/PositionIterator_test.cpp 
                     EXECUTABLE Raster_PositionIterator_test
                     LINK_LIBRARIES Raster
                     TYPE Boost)
elements_add_unit_test(Raster tests/src/Raster_test.cpp 
                     EXECUTABLE Raster_Raster_test
                     LINK_LIBRARIES Raster
                     TYPE Boost)
elements_add_unit_test(Region tests/src/Region_test.cpp 
                     EXECUTABLE Raster_Region_test
                     LINK_LIBRARIES Raster
                     TYPE Boost)
elements_add_unit_test(Subraster tests/src/Subraster_test.cpp 
                     EXECUTABLE Raster_Subraster_test
                     LINK_LIBRARIES Raster
                     TYPE Boost)
elements_add_unit_test(VectorArithmetic tests/src/VectorArithmetic_test.cpp 
                     EXECUTABLE Raster_VectorArithmetic_test
                     LINK_LIBRARIES Raster
                     TYPE Boost)
elements_add_unit_test(TestRaster tests/src/TestRaster_test.cpp 
                     EXECUTABLE Raster_TestRaster_test
                     LINK_LIBRARIES Raster
                     TYPE Boost)
elements_add_unit_test(TestUtils tests/src/TestUtils_test.cpp 
                     EXECUTABLE Raster_TestUtils_test
                     LINK_LIBRARIES Raster
                     TYPE Boost)
