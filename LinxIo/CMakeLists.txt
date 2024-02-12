CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

elements_subdir(LinxIo)

elements_depends_on_subdirs(Linx)

find_package(Boost) # test

elements_add_unit_test(Fits tests/src/Fits_test.cpp 
                     EXECUTABLE LinxIo_Fits_test
                     LINK_LIBRARIES Linx
                     TYPE Boost)
