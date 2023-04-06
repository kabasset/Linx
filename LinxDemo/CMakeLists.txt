CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

elements_subdir(LinxDemo)

elements_depends_on_subdirs(LinxCore LinxTransforms LinxTransforms)

elements_add_unit_test(LinxDemoBasics tests/src/LinxDemoBasics_test.cpp 
                     EXECUTABLE LinxDemoBasics_test
                     LINK_LIBRARIES LinxCore
                     TYPE Boost)
elements_add_unit_test(LinxDemoChannels tests/src/LinxDemoChannels_test.cpp 
                     EXECUTABLE LinxDemoChannels_test
                     LINK_LIBRARIES LinxCore
                     TYPE Boost)
elements_add_unit_test(LinxDemoConstructors tests/src/LinxDemoConstructors_test.cpp 
                     EXECUTABLE LinxDemoConstructors_test
                     LINK_LIBRARIES LinxCore
                     TYPE Boost)
elements_add_unit_test(LinxDemoDft tests/src/LinxDemoDft_test.cpp 
                     EXECUTABLE LinxDemoDft_test
                     LINK_LIBRARIES LinxTransforms
                     TYPE Boost)
elements_add_unit_test(LinxDemoInterpolation tests/src/LinxDemoInterpolation_test.cpp 
                     EXECUTABLE LinxDemoInterpolation_test
                     LINK_LIBRARIES LinxTransforms
                     TYPE Boost)
elements_add_unit_test(LinxDemoPixelwise tests/src/LinxDemoPixelwise_test.cpp 
                     EXECUTABLE LinxDemoPixelwise_test
                     LINK_LIBRARIES LinxCore
                     TYPE Boost)
elements_add_unit_test(LinxDemoRandom tests/src/LinxDemoRandom_test.cpp 
                     EXECUTABLE LinxDemoRandom_test
                     LINK_LIBRARIES LinxCore
                     TYPE Boost)
