CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

elements_subdir(Linx)

find_package(Boost) # Base
find_package(Eigen3) # Transforms
find_package(FFTW) # Transforms

elements_add_library(Linx src/lib/*.cpp
                     INCLUDE_DIRS Boost Eigen3 FFTW
                     LINK_LIBRARIES Boost Eigen3 FFTW
                     PUBLIC_HEADERS Linx)
