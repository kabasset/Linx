CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

elements_subdir(LinxData)

elements_depends_on_subdirs(Linx)

find_package(Boost)

elements_add_unit_test(BorderedBox tests/src/BorderedBox_test.cpp 
                     EXECUTABLE LinxData_BorderedBox_test
                     LINK_LIBRARIES Linx
                     TYPE Boost)
elements_add_unit_test(Box tests/src/Box_test.cpp 
                     EXECUTABLE LinxData_Box_test
                     LINK_LIBRARIES Linx
                     TYPE Boost)
elements_add_unit_test(BoxIterator tests/src/BoxIterator_test.cpp 
                     EXECUTABLE LinxData_BoxIterator_test
                     LINK_LIBRARIES Linx
                     TYPE Boost)
elements_add_unit_test(Grid tests/src/Grid_test.cpp 
                     EXECUTABLE LinxData_Grid_test
                     LINK_LIBRARIES Linx
                     TYPE Boost)
elements_add_unit_test(Mask tests/src/Mask_test.cpp 
                     EXECUTABLE LinxData_Mask_test
                     LINK_LIBRARIES Linx
                     TYPE Boost)
elements_add_unit_test(LineIterator tests/src/LineIterator_test.cpp 
                     EXECUTABLE LinxData_LineIterator_test
                     LINK_LIBRARIES Linx
                     TYPE Boost)
elements_add_unit_test(Patch tests/src/Patch_test.cpp 
                     EXECUTABLE LinxData_Patch_test
                     LINK_LIBRARIES Linx
                     TYPE Boost)
elements_add_unit_test(PatchIterator tests/src/PatchIterator_test.cpp 
                     EXECUTABLE LinxData_PatchIterator_test
                     LINK_LIBRARIES Linx
                     TYPE Boost)
elements_add_unit_test(Raster tests/src/Raster_test.cpp 
                     EXECUTABLE LinxData_Raster_test
                     LINK_LIBRARIES Linx
                     TYPE Boost)
elements_add_unit_test(Sequence tests/src/Sequence_test.cpp 
                     EXECUTABLE LinxData_Sequence_test
                     LINK_LIBRARIES Linx
                     TYPE Boost)
elements_add_unit_test(Tiling tests/src/Tiling_test.cpp 
                    EXECUTABLE LinxData_Tiling_test
                    LINK_LIBRARIES Linx
                    TYPE Boost)
elements_add_unit_test(Vector tests/src/Vector_test.cpp 
                    EXECUTABLE LinxData_Vector_test
                    LINK_LIBRARIES Linx
                    TYPE Boost)
