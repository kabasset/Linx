CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

elements_subdir(KRasterFourier)

elements_depends_on_subdirs(KRaster)

find_package(FFTW)

elements_add_library(KRasterFourier src/lib/*.cpp
                     INCLUDE_DIRS KRaster FFTW
                     LINK_LIBRARIES KRaster FFTW
                     PUBLIC_HEADERS KRasterFourier)

elements_add_unit_test(Dft tests/src/Dft_test.cpp 
                     EXECUTABLE KRasterFourier_Dft_test
                     LINK_LIBRARIES KRasterFourier
                     TYPE Boost)
elements_add_unit_test(DftMemory tests/src/DftMemory_test.cpp 
                     EXECUTABLE KRasterFourier_DftMemory_test
                     LINK_LIBRARIES KRasterFourier
                     TYPE Boost)
elements_add_unit_test(DftBuffer tests/src/DftBuffer_test.cpp 
                     EXECUTABLE KRasterFourier_DftBuffer_test
                     LINK_LIBRARIES KRasterFourier
                     TYPE Boost)
elements_add_unit_test(DftPlan tests/src/DftPlan_test.cpp 
                     EXECUTABLE KRasterFourier_DftPlan_test
                     LINK_LIBRARIES KRasterFourier
                     TYPE Boost)
