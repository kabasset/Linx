CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

elements_subdir(RasterDemo)

elements_depends_on_subdirs(Raster RasterFourier RasterGeometry)

elements_add_unit_test(RasterDemoBasics tests/src/RasterDemoBasics_test.cpp 
                     EXECUTABLE RasterDemoBasics_test
                     LINK_LIBRARIES Raster
                     TYPE Boost)
elements_add_unit_test(RasterDemoChannels tests/src/RasterDemoChannels_test.cpp 
                     EXECUTABLE RasterDemoChannels_test
                     LINK_LIBRARIES Raster
                     TYPE Boost)
elements_add_unit_test(RasterDemoConstructors tests/src/RasterDemoConstructors_test.cpp 
                     EXECUTABLE RasterDemoConstructors_test
                     LINK_LIBRARIES Raster
                     TYPE Boost)
elements_add_unit_test(RasterDemoDft tests/src/RasterDemoDft_test.cpp 
                     EXECUTABLE RasterDemoDft_test
                     LINK_LIBRARIES RasterFourier
                     TYPE Boost)
elements_add_unit_test(RasterDemoInterpolation tests/src/RasterDemoInterpolation_test.cpp 
                     EXECUTABLE RasterDemoInterpolation_test
                     LINK_LIBRARIES RasterGeometry
                     TYPE Boost)
elements_add_unit_test(RasterDemoPixelwise tests/src/RasterDemoPixelwise_test.cpp 
                     EXECUTABLE RasterDemoPixelwise_test
                     LINK_LIBRARIES Raster
                     TYPE Boost)
elements_add_unit_test(RasterDemoRandom tests/src/RasterDemoRandom_test.cpp 
                     EXECUTABLE RasterDemoRandom_test
                     LINK_LIBRARIES Raster
                     TYPE Boost)
