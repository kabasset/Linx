CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

elements_subdir(LitlContainer)

elements_depends_on_subdirs(LitlTypes)

find_package(Boost) # Operators

elements_add_library(LitlContainer src/lib/*.cpp
                     INCLUDE_DIRS LitlTypes Boost
                     LINK_LIBRARIES LitlTypes Boost
                     PUBLIC_HEADERS LitlContainer)

elements_add_unit_test(AlignedBuffer tests/src/AlignedBuffer_test.cpp 
                     EXECUTABLE LitlContainer_AlignedBuffer_test
                     LINK_LIBRARIES LitlContainer
                     TYPE Boost)
elements_add_unit_test(Arithmetic tests/src/Arithmetic_test.cpp 
                     EXECUTABLE LitlContainer_Arithmetic_test
                     LINK_LIBRARIES LitlContainer
                     TYPE Boost)
elements_add_unit_test(ContiguousContainer tests/src/ContiguousContainer_test.cpp 
                     EXECUTABLE LitlContainer_ContiguousContainer_test
                     LINK_LIBRARIES LitlContainer
                     TYPE Boost)
elements_add_unit_test(DataContainer tests/src/DataContainer_test.cpp 
                     EXECUTABLE LitlContainer_DataContainer_test
                     LINK_LIBRARIES LitlContainer
                     TYPE Boost)
elements_add_unit_test(DataDistribution tests/src/DataDistribution_test.cpp 
                     EXECUTABLE LitlContainer_DataDistribution_test
                     LINK_LIBRARIES LitlContainer
                     TYPE Boost)
elements_add_unit_test(Holders tests/src/Holders_test.cpp 
                     EXECUTABLE LitlContainer_Holders_test
                     LINK_LIBRARIES LitlContainer
                     TYPE Boost)
elements_add_unit_test(Math tests/src/Math_test.cpp 
                     EXECUTABLE LitlContainer_Math_test
                     LINK_LIBRARIES LitlContainer
                     TYPE Boost)
elements_add_unit_test(Random tests/src/Random_test.cpp 
                     EXECUTABLE LitlContainer_Random_test
                     LINK_LIBRARIES LitlContainer
                     TYPE Boost)
elements_add_unit_test(Sequence tests/src/Sequence_test.cpp 
                     EXECUTABLE LitlContainer_Sequence_test
                     LINK_LIBRARIES LitlContainer
                     TYPE Boost)
