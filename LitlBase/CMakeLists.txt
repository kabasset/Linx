CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

elements_subdir(LitlBase)

find_package(Boost) # Operators

elements_add_library(LitlBase src/lib/*.cpp
                     INCLUDE_DIRS Boost
                     LINK_LIBRARIES Boost
                     PUBLIC_HEADERS LitlBase)

elements_add_unit_test(AlignedBuffer tests/src/AlignedBuffer_test.cpp 
                     EXECUTABLE LitlBase_AlignedBuffer_test
                     LINK_LIBRARIES LitlBase
                     TYPE Boost)
elements_add_unit_test(Arithmetic tests/src/Arithmetic_test.cpp 
                     EXECUTABLE LitlBase_Arithmetic_test
                     LINK_LIBRARIES LitlBase
                     TYPE Boost)
elements_add_unit_test(ContiguousContainer tests/src/ContiguousContainer_test.cpp 
                     EXECUTABLE LitlBase_ContiguousContainer_test
                     LINK_LIBRARIES LitlBase
                     TYPE Boost)
elements_add_unit_test(DataContainer tests/src/DataContainer_test.cpp 
                     EXECUTABLE LitlBase_DataContainer_test
                     LINK_LIBRARIES LitlBase
                     TYPE Boost)
elements_add_unit_test(DataDistribution tests/src/DataDistribution_test.cpp 
                     EXECUTABLE LitlBase_DataDistribution_test
                     LINK_LIBRARIES LitlBase
                     TYPE Boost)
elements_add_unit_test(Exceptions tests/src/Exceptions_test.cpp 
                     EXECUTABLE LitlBase_Exceptions_test
                     LINK_LIBRARIES LitlBase
                     TYPE Boost)
elements_add_unit_test(Holders tests/src/Holders_test.cpp 
                     EXECUTABLE LitlBase_Holders_test
                     LINK_LIBRARIES LitlBase
                     TYPE Boost)
elements_add_unit_test(Math tests/src/Math_test.cpp 
                     EXECUTABLE LitlBase_Math_test
                     LINK_LIBRARIES LitlBase
                     TYPE Boost)
elements_add_unit_test(Random tests/src/Random_test.cpp 
                     EXECUTABLE LitlBase_Random_test
                     LINK_LIBRARIES LitlBase
                     TYPE Boost)
elements_add_unit_test(SeqUtils tests/src/SeqUtils_test.cpp 
                     EXECUTABLE LitlBase_SeqUtils_test
                     LINK_LIBRARIES LitlBase
                     TYPE Boost)
elements_add_unit_test(Slice tests/src/Slice_test.cpp 
                     EXECUTABLE LitlBase_Slice_test
                     LINK_LIBRARIES LitlBase
                     TYPE Boost)
elements_add_unit_test(TypeUtils tests/src/TypeUtils_test.cpp 
                     EXECUTABLE LitlBase_TypeUtils_test
                     LINK_LIBRARIES LitlBase
                     TYPE Boost)
