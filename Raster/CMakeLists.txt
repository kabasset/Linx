CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

elements_subdir(Raster)

elements_depends_on_subdirs(LitlContainer)

find_package(Boost)

elements_add_library(Raster src/lib/*.cpp
                     INCLUDE_DIRS LitlContainer Boost
                     LINK_LIBRARIES LitlContainer Boost
                     PUBLIC_HEADERS Raster)

elements_add_unit_test(Position tests/src/Position_test.cpp 
                     EXECUTABLE Raster_Position_test
                     LINK_LIBRARIES Raster
                     TYPE Boost)
elements_add_unit_test(PositionIterator tests/src/PositionIterator_test.cpp 
                     EXECUTABLE Raster_PositionIterator_test
                     LINK_LIBRARIES Raster
                     TYPE Boost)
elements_add_unit_test(Raster tests/src/Raster_test.cpp 
                     EXECUTABLE Raster_Raster_test
                     LINK_LIBRARIES Raster
                     TYPE Boost)
elements_add_unit_test(Region tests/src/Region_test.cpp 
                     EXECUTABLE Raster_Region_test
                     LINK_LIBRARIES Raster
                     TYPE Boost)
elements_add_unit_test(Subraster tests/src/Subraster_test.cpp 
                     EXECUTABLE Raster_Subraster_test
                     LINK_LIBRARIES Raster
                     TYPE Boost)
