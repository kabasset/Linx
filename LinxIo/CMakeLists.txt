CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

elements_subdir(LinxIo)

elements_depends_on_subdirs(LinxCore)

find_package(Cfitsio)
#find_package(Libpng)
#find_package(Libtiff)

elements_add_library(LinxIo src/lib/*.cpp
                     INCLUDE_DIRS LinxCore Cfitsio
                     LINK_LIBRARIES LinxCore Cfitsio
                     PUBLIC_HEADERS LinxIo)

elements_add_unit_test(Fits tests/src/Fits_test.cpp 
                     EXECUTABLE LinxIo_Fits_test
                     LINK_LIBRARIES LinxIo
                     TYPE Boost)
