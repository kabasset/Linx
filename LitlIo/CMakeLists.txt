CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

elements_subdir(LitlIo)

elements_depends_on_subdirs(LitlCore)

find_package(Cfitsio)
#find_package(Libpng)
#find_package(Libtiff)

elements_add_library(LitlIo src/lib/*.cpp
                     INCLUDE_DIRS LitlCore Cfitsio
                     LINK_LIBRARIES LitlCore Cfitsio
                     PUBLIC_HEADERS LitlIo)

elements_add_unit_test(Fits tests/src/Fits_test.cpp 
                     EXECUTABLE LitlIo_Fits_test
                     LINK_LIBRARIES LitlIo
                     TYPE Boost)
