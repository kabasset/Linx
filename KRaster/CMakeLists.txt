CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

elements_subdir(KRaster)

elements_depends_on_subdirs(KRasterUtils)
elements_depends_on_subdirs(ElementsKernel)

find_package(Boost)
elements_add_library(KRaster src/lib/*.cpp
                     INCLUDE_DIRS KRasterUtils Boost ElementsKernel
                     LINK_LIBRARIES KRasterUtils Boost ElementsKernel
                     PUBLIC_HEADERS KRaster)
elements_add_unit_test(ContiguousContainer tests/src/ContiguousContainer_test.cpp 
                     EXECUTABLE KRaster_ContiguousContainer_test
                     LINK_LIBRARIES KRaster
                     TYPE Boost)
elements_add_unit_test(DataContainer tests/src/DataContainer_test.cpp 
                     EXECUTABLE KRaster_DataContainer_test
                     LINK_LIBRARIES KRaster
                     TYPE Boost)
elements_add_unit_test(DataUtils tests/src/DataUtils_test.cpp 
                     EXECUTABLE KRaster_DataUtils_test
                     LINK_LIBRARIES KRaster
                     TYPE Boost)
elements_add_unit_test(Position tests/src/Position_test.cpp 
                     EXECUTABLE KRaster_Position_test
                     LINK_LIBRARIES KRaster
                     TYPE Boost)
elements_add_unit_test(PositionIterator tests/src/PositionIterator_test.cpp 
                     EXECUTABLE KRaster_PositionIterator_test
                     LINK_LIBRARIES KRaster
                     TYPE Boost)
elements_add_unit_test(Raster tests/src/Raster_test.cpp 
                     EXECUTABLE KRaster_Raster_test
                     LINK_LIBRARIES KRaster
                     TYPE Boost)
elements_add_unit_test(Region tests/src/Region_test.cpp 
                     EXECUTABLE KRaster_Region_test
                     LINK_LIBRARIES KRaster
                     TYPE Boost)
elements_add_unit_test(Subraster tests/src/Subraster_test.cpp 
                     EXECUTABLE KRaster_Subraster_test
                     LINK_LIBRARIES KRaster
                     TYPE Boost)
elements_add_unit_test(VectorArithmetic tests/src/VectorArithmetic_test.cpp 
                     EXECUTABLE KRaster_VectorArithmetic_test
                     LINK_LIBRARIES KRaster
                     TYPE Boost)
elements_add_unit_test(TestRaster tests/src/TestRaster_test.cpp 
                     EXECUTABLE KRaster_TestRaster_test
                     LINK_LIBRARIES KRaster
                     TYPE Boost)
elements_add_unit_test(TestUtils tests/src/TestUtils_test.cpp 
                     EXECUTABLE KRaster_TestUtils_test
                     LINK_LIBRARIES KRaster
                     TYPE Boost)
