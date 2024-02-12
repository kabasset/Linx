CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

elements_subdir(LinxTransforms)

elements_depends_on_subdirs(Linx)

find_package(Boost) # test

elements_add_unit_test(Affinity tests/src/Affinity_test.cpp 
                     EXECUTABLE LinxTransforms_Affinity_test
                     LINK_LIBRARIES Linx
                     TYPE Boost)
elements_add_unit_test(Dft tests/src/Dft_test.cpp 
                     EXECUTABLE LinxTransforms_Dft_test
                     LINK_LIBRARIES Linx
                     TYPE Boost)
elements_add_unit_test(DftMemory tests/src/DftMemory_test.cpp 
                     EXECUTABLE LinxTransforms_DftMemory_test
                     LINK_LIBRARIES Linx
                     TYPE Boost)
elements_add_unit_test(DftPlan tests/src/DftPlan_test.cpp 
                     EXECUTABLE LinxTransforms_DftPlan_test
                     LINK_LIBRARIES Linx
                     TYPE Boost)
elements_add_unit_test(FilterAgg tests/src/FilterAgg_test.cpp 
                     EXECUTABLE LinxTransforms_FilterAgg_test
                     LINK_LIBRARIES Linx
                     TYPE Boost)
elements_add_unit_test(FilterSeq tests/src/FilterSeq_test.cpp 
                     EXECUTABLE LinxTransforms_FilterSeq_test
                     LINK_LIBRARIES Linx
                     TYPE Boost)
elements_add_unit_test(Filters tests/src/Filters_test.cpp 
                     EXECUTABLE LinxTransforms_Filters_test
                     LINK_LIBRARIES Linx
                     TYPE Boost)
elements_add_unit_test(Interpolation tests/src/Interpolation_test.cpp 
                     EXECUTABLE LinxTransforms_Interpolation_test
                     LINK_LIBRARIES Linx
                     TYPE Boost)
elements_add_unit_test(SimpleFilter tests/src/SimpleFilter_test.cpp 
                     EXECUTABLE LinxTransforms_SimpleFilter_test
                     LINK_LIBRARIES Linx
                     TYPE Boost)
