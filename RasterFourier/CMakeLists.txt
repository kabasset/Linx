CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

elements_subdir(RasterFourier)

elements_depends_on_subdirs(Raster)

find_package(FFTW)

elements_add_library(RasterFourier src/lib/*.cpp
                     INCLUDE_DIRS Raster FFTW
                     LINK_LIBRARIES Raster FFTW
                     PUBLIC_HEADERS RasterFourier)

elements_add_unit_test(Dft tests/src/Dft_test.cpp 
                     EXECUTABLE RasterFourier_Dft_test
                     LINK_LIBRARIES RasterFourier
                     TYPE Boost)
elements_add_unit_test(DftBuffer tests/src/DftBuffer_test.cpp 
                     EXECUTABLE RasterFourier_DftBuffer_test
                     LINK_LIBRARIES RasterFourier
                     TYPE Boost)
elements_add_unit_test(DftMemory tests/src/DftMemory_test.cpp 
                     EXECUTABLE RasterFourier_DftMemory_test
                     LINK_LIBRARIES RasterFourier
                     TYPE Boost)
elements_add_unit_test(DftPlan tests/src/DftPlan_test.cpp 
                     EXECUTABLE RasterFourier_DftPlan_test
                     LINK_LIBRARIES RasterFourier
                     TYPE Boost)
