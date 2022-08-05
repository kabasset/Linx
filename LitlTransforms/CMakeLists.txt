CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

elements_subdir(LitlTransforms)

elements_depends_on_subdirs(LitlRaster)

find_package(Eigen3)
find_package(FFTW)

elements_add_library(LitlTransforms src/lib/*.cpp
                     INCLUDE_DIRS LitlRaster Eigen3 FFTW
                     LINK_LIBRARIES LitlRaster Eigen3 FFTW
                     PUBLIC_HEADERS LitlTransforms)

elements_add_unit_test(Affinity tests/src/Affinity_test.cpp 
                     EXECUTABLE LitlTransforms_Affinity_test
                     LINK_LIBRARIES LitlTransforms
                     TYPE Boost)
elements_add_unit_test(Convolution tests/src/Convolution_test.cpp 
                     EXECUTABLE LitlTransforms_Convolution_test
                     LINK_LIBRARIES LitlTransforms
                     TYPE Boost)
elements_add_unit_test(Dft tests/src/Dft_test.cpp 
                     EXECUTABLE LitlTransforms_Dft_test
                     LINK_LIBRARIES LitlTransforms
                     TYPE Boost)
elements_add_unit_test(DftMemory tests/src/DftMemory_test.cpp 
                     EXECUTABLE LitlTransforms_DftMemory_test
                     LINK_LIBRARIES LitlTransforms
                     TYPE Boost)
elements_add_unit_test(DftPlan tests/src/DftPlan_test.cpp 
                     EXECUTABLE LitlTransforms_DftPlan_test
                     LINK_LIBRARIES LitlTransforms
                     TYPE Boost)
elements_add_unit_test(Interpolation tests/src/Interpolation_test.cpp 
                     EXECUTABLE LitlTransforms_Interpolation_test
                     LINK_LIBRARIES LitlTransforms
                     TYPE Boost)
elements_add_unit_test(Kernel tests/src/Kernel_test.cpp 
                     EXECUTABLE LitlTransforms_Kernel_test
                     LINK_LIBRARIES LitlTransforms
                     TYPE Boost)
elements_add_unit_test(LineKernel tests/src/LineKernel_test.cpp 
                     EXECUTABLE LitlTransforms_LineKernel_test
                     LINK_LIBRARIES LitlTransforms
                     TYPE Boost)
elements_add_unit_test(OrientedKernel tests/src/OrientedKernel_test.cpp 
                     EXECUTABLE LitlTransforms_OrientedKernel_test
                     LINK_LIBRARIES LitlTransforms
                     TYPE Boost)
elements_add_unit_test(SeparableKernel tests/src/SeparableKernel_test.cpp 
                     EXECUTABLE LitlTransforms_SeparableKernel_test
                     LINK_LIBRARIES LitlTransforms
                     TYPE Boost)
elements_add_unit_test(StructuringElement tests/src/StructuringElement_test.cpp 
                     EXECUTABLE LitlTransforms_StructuringElement_test
                     LINK_LIBRARIES LitlTransforms
                     TYPE Boost)
