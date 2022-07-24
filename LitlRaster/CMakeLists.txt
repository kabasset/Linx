CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

elements_subdir(LitlRaster)

elements_depends_on_subdirs(LitlContainer)

find_package(Boost)

elements_add_library(LitlRaster src/lib/*.cpp
                     INCLUDE_DIRS LitlContainer Boost
                     LINK_LIBRARIES LitlContainer Boost
                     PUBLIC_HEADERS LitlRaster)

elements_add_unit_test(Ball tests/src/Ball_test.cpp 
                     EXECUTABLE LitlRaster_Ball_test
                     LINK_LIBRARIES LitlRaster
                     TYPE Boost)
elements_add_unit_test(BallIterator tests/src/BallIterator_test.cpp 
                     EXECUTABLE LitlRaster_BallIterator_test
                     LINK_LIBRARIES LitlRaster
                     TYPE Boost)
elements_add_unit_test(Box tests/src/Box_test.cpp 
                     EXECUTABLE LitlRaster_Box_test
                     LINK_LIBRARIES LitlRaster
                     TYPE Boost)
elements_add_unit_test(BoxIterator tests/src/BoxIterator_test.cpp 
                     EXECUTABLE LitlRaster_BoxIterator_test
                     LINK_LIBRARIES LitlRaster
                     TYPE Boost)
elements_add_unit_test(Mask tests/src/Mask_test.cpp 
                     EXECUTABLE LitlRaster_Mask_test
                     LINK_LIBRARIES LitlRaster
                     TYPE Boost)
elements_add_unit_test(Raster tests/src/Raster_test.cpp 
                     EXECUTABLE LitlRaster_Raster_test
                     LINK_LIBRARIES LitlRaster
                     TYPE Boost)
elements_add_unit_test(Subraster tests/src/Subraster_test.cpp 
                     EXECUTABLE LitlRaster_Subraster_test
                     LINK_LIBRARIES LitlRaster
                     TYPE Boost)
elements_add_unit_test(SubrasterIterator tests/src/SubrasterIterator_test.cpp 
                     EXECUTABLE LitlRaster_SubrasterIterator_test
                     LINK_LIBRARIES LitlRaster
                     TYPE Boost)
elements_add_unit_test(Vector tests/src/Vector_test.cpp 
                     EXECUTABLE LitlRaster_Vector_test
                     LINK_LIBRARIES LitlRaster
                     TYPE Boost)
