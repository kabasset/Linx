CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

elements_subdir(LinxCore)

elements_depends_on_subdirs(LinxBase)

find_package(Boost)

elements_add_library(LinxCore src/lib/*.cpp
                     INCLUDE_DIRS LinxBase Boost
                     LINK_LIBRARIES LinxBase Boost
                     PUBLIC_HEADERS LinxCore)

elements_add_unit_test(BorderedBox tests/src/BorderedBox_test.cpp 
                     EXECUTABLE LinxCore_BorderedBox_test
                     LINK_LIBRARIES LinxCore
                     TYPE Boost)
elements_add_unit_test(Box tests/src/Box_test.cpp 
                     EXECUTABLE LinxCore_Box_test
                     LINK_LIBRARIES LinxCore
                     TYPE Boost)
elements_add_unit_test(BoxIterator tests/src/BoxIterator_test.cpp 
                     EXECUTABLE LinxCore_BoxIterator_test
                     LINK_LIBRARIES LinxCore
                     TYPE Boost)
elements_add_unit_test(Grid tests/src/Grid_test.cpp 
                     EXECUTABLE LinxCore_Grid_test
                     LINK_LIBRARIES LinxCore
                     TYPE Boost)
elements_add_unit_test(Mask tests/src/Mask_test.cpp 
                     EXECUTABLE LinxCore_Mask_test
                     LINK_LIBRARIES LinxCore
                     TYPE Boost)
elements_add_unit_test(OrientedSliceIterator tests/src/OrientedSliceIterator_test.cpp 
                     EXECUTABLE LinxCore_OrientedSliceIterator_test
                     LINK_LIBRARIES LinxCore
                     TYPE Boost)
elements_add_unit_test(Patch tests/src/Patch_test.cpp 
                     EXECUTABLE LinxCore_Patch_test
                     LINK_LIBRARIES LinxCore
                     TYPE Boost)
elements_add_unit_test(PatchIterator tests/src/PatchIterator_test.cpp 
                     EXECUTABLE LinxCore_PatchIterator_test
                     LINK_LIBRARIES LinxCore
                     TYPE Boost)
elements_add_unit_test(Raster tests/src/Raster_test.cpp 
                     EXECUTABLE LinxCore_Raster_test
                     LINK_LIBRARIES LinxCore
                     TYPE Boost)
elements_add_unit_test(Sequence tests/src/Sequence_test.cpp 
                     EXECUTABLE LinxCore_Sequence_test
                     LINK_LIBRARIES LinxCore
                     TYPE Boost)
elements_add_unit_test(Vector tests/src/Vector_test.cpp 
                     EXECUTABLE LinxCore_Vector_test
                     LINK_LIBRARIES LinxCore
                     TYPE Boost)
