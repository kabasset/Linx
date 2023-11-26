CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

elements_subdir(LinxTransforms)

elements_depends_on_subdirs(Linx)

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
elements_add_unit_test(Interpolation tests/src/Interpolation_test.cpp 
                     EXECUTABLE LinxTransforms_Interpolation_test
                     LINK_LIBRARIES Linx
                     TYPE Boost)
elements_add_unit_test(Kernel tests/src/Kernel_test.cpp 
                     EXECUTABLE LinxTransforms_Kernel_test
                     LINK_LIBRARIES Linx
                     TYPE Boost)
elements_add_unit_test(OrientedKernel tests/src/OrientedKernel_test.cpp 
                     EXECUTABLE LinxTransforms_OrientedKernel_test
                     LINK_LIBRARIES Linx
                     TYPE Boost)
elements_add_unit_test(SeparableKernel tests/src/SeparableKernel_test.cpp 
                     EXECUTABLE LinxTransforms_SeparableKernel_test
                     LINK_LIBRARIES Linx
                     TYPE Boost)
elements_add_unit_test(Filter tests/src/Filter_test.cpp 
                     EXECUTABLE LinxTransforms_Filter_test
                     LINK_LIBRARIES Linx
                     TYPE Boost)
