CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

elements_subdir(LitlTypes)

# elements_depends_on_subdirs(ElementsKernel)

elements_add_library(LitlTypes src/lib/*.cpp
                     PUBLIC_HEADERS LitlTypes)

elements_add_unit_test(Exceptions tests/src/Exceptions_test.cpp 
                     EXECUTABLE LitlTypes_Exceptions_test
                     LINK_LIBRARIES LitlTypes
                     TYPE Boost)
elements_add_unit_test(SeqUtils tests/src/SeqUtils_test.cpp 
                     EXECUTABLE LitlTypes_SeqUtils_test
                     LINK_LIBRARIES LitlTypes
                     TYPE Boost)
elements_add_unit_test(TypeUtils tests/src/TypeUtils_test.cpp 
                     EXECUTABLE LitlTypes_TypeUtils_test
                     LINK_LIBRARIES LitlTypes
                     TYPE Boost)
