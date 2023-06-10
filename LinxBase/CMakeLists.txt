CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

elements_subdir(LinxBase)

find_package(Boost) # Operators

elements_add_unit_test(AlignedBuffer tests/src/AlignedBuffer_test.cpp 
                     EXECUTABLE LinxBase_AlignedBuffer_test
                     LINK_LIBRARIES Linx
                     TYPE Boost)
elements_add_unit_test(Arithmetic tests/src/Arithmetic_test.cpp 
                     EXECUTABLE LinxBase_Arithmetic_test
                     LINK_LIBRARIES Linx
                     TYPE Boost)
elements_add_unit_test(ContiguousContainer tests/src/ContiguousContainer_test.cpp 
                     EXECUTABLE LinxBase_ContiguousContainer_test
                     LINK_LIBRARIES Linx
                     TYPE Boost)
elements_add_unit_test(DataContainer tests/src/DataContainer_test.cpp 
                     EXECUTABLE LinxBase_DataContainer_test
                     LINK_LIBRARIES Linx
                     TYPE Boost)
elements_add_unit_test(DataDistribution tests/src/DataDistribution_test.cpp 
                     EXECUTABLE LinxBase_DataDistribution_test
                     LINK_LIBRARIES Linx
                     TYPE Boost)
elements_add_unit_test(Exceptions tests/src/Exceptions_test.cpp 
                     EXECUTABLE LinxBase_Exceptions_test
                     LINK_LIBRARIES Linx
                     TYPE Boost)
elements_add_unit_test(Holders tests/src/Holders_test.cpp 
                     EXECUTABLE LinxBase_Holders_test
                     LINK_LIBRARIES Linx
                     TYPE Boost)
elements_add_unit_test(Math tests/src/Math_test.cpp 
                     EXECUTABLE LinxBase_Math_test
                     LINK_LIBRARIES Linx
                     TYPE Boost)
elements_add_unit_test(Random tests/src/Random_test.cpp 
                     EXECUTABLE LinxBase_Random_test
                     LINK_LIBRARIES Linx
                     TYPE Boost)
elements_add_unit_test(SeqUtils tests/src/SeqUtils_test.cpp 
                     EXECUTABLE LinxBase_SeqUtils_test
                     LINK_LIBRARIES Linx
                     TYPE Boost)
elements_add_unit_test(Slice tests/src/Slice_test.cpp 
                     EXECUTABLE LinxBase_Slice_test
                     LINK_LIBRARIES Linx
                     TYPE Boost)
elements_add_unit_test(TypeUtils tests/src/TypeUtils_test.cpp 
                     EXECUTABLE LinxBase_TypeUtils_test
                     LINK_LIBRARIES Linx
                     TYPE Boost)
