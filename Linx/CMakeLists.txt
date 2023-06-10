CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

elements_subdir(Linx)

find_package(Boost) # Operators

elements_add_library(Linx src/lib/*.cpp
                     INCLUDE_DIRS Boost
                     LINK_LIBRARIES Boost
                     PUBLIC_HEADERS Linx)
