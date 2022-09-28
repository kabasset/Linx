CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

elements_subdir(LitlRaster)

elements_depends_on_subdirs(LitlContainer)

find_package(Boost)

elements_add_library(LitlRaster src/lib/*.cpp
                     INCLUDE_DIRS LitlContainer Boost
                     LINK_LIBRARIES LitlContainer Boost
                     PUBLIC_HEADERS LitlRaster)

elements_add_unit_test(BorderedBox tests/src/BorderedBox_test.cpp 
                     EXECUTABLE LitlRaster_BorderedBox_test
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
elements_add_unit_test(Grid tests/src/Grid_test.cpp 
                     EXECUTABLE LitlRaster_Grid_test
                     LINK_LIBRARIES LitlRaster
                     TYPE Boost)
elements_add_unit_test(Mask tests/src/Mask_test.cpp 
                     EXECUTABLE LitlRaster_Mask_test
                     LINK_LIBRARIES LitlRaster
                     TYPE Boost)
elements_add_unit_test(OrientedSliceIterator tests/src/OrientedSliceIterator_test.cpp 
                     EXECUTABLE LitlRaster_OrientedSliceIterator_test
                     LINK_LIBRARIES LitlRaster
                     TYPE Boost)
elements_add_unit_test(Raster tests/src/Raster_test.cpp 
                     EXECUTABLE LitlRaster_Raster_test
                     LINK_LIBRARIES LitlRaster
                     TYPE Boost)
elements_add_unit_test(Patch tests/src/Patch_test.cpp 
                     EXECUTABLE LitlRaster_Patch_test
                     LINK_LIBRARIES LitlRaster
                     TYPE Boost)
elements_add_unit_test(PatchIterator tests/src/PatchIterator_test.cpp 
                     EXECUTABLE LitlRaster_PatchIterator_test
                     LINK_LIBRARIES LitlRaster
                     TYPE Boost)
elements_add_unit_test(Vector tests/src/Vector_test.cpp 
                     EXECUTABLE LitlRaster_Vector_test
                     LINK_LIBRARIES LitlRaster
                     TYPE Boost)
