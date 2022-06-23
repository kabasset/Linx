CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

elements_subdir(LitlDemo)

elements_depends_on_subdirs(LitlRaster RasterFourier LitlTransforms)

elements_add_unit_test(LitlDemoBasics tests/src/LitlDemoBasics_test.cpp 
                     EXECUTABLE LitlDemoBasics_test
                     LINK_LIBRARIES LitlRaster
                     TYPE Boost)
elements_add_unit_test(LitlDemoChannels tests/src/LitlDemoChannels_test.cpp 
                     EXECUTABLE LitlDemoChannels_test
                     LINK_LIBRARIES LitlRaster
                     TYPE Boost)
elements_add_unit_test(LitlDemoConstructors tests/src/LitlDemoConstructors_test.cpp 
                     EXECUTABLE LitlDemoConstructors_test
                     LINK_LIBRARIES LitlRaster
                     TYPE Boost)
elements_add_unit_test(LitlDemoDft tests/src/LitlDemoDft_test.cpp 
                     EXECUTABLE LitlDemoDft_test
                     LINK_LIBRARIES RasterFourier
                     TYPE Boost)
elements_add_unit_test(LitlDemoInterpolation tests/src/LitlDemoInterpolation_test.cpp 
                     EXECUTABLE LitlDemoInterpolation_test
                     LINK_LIBRARIES LitlTransforms
                     TYPE Boost)
elements_add_unit_test(LitlDemoPixelwise tests/src/LitlDemoPixelwise_test.cpp 
                     EXECUTABLE LitlDemoPixelwise_test
                     LINK_LIBRARIES LitlRaster
                     TYPE Boost)
elements_add_unit_test(LitlDemoRandom tests/src/LitlDemoRandom_test.cpp 
                     EXECUTABLE LitlDemoRandom_test
                     LINK_LIBRARIES LitlRaster
                     TYPE Boost)
