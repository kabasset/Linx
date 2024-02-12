CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

elements_subdir(Linx)

find_package(Boost REQUIRED COMPONENTS program_options) # Base: operators (header only, thus no COMPONENTS); Run: program_options
find_package(Cfitsio REQUIRED) # Io
find_package(Eigen3 REQUIRED) # Transforms
find_package(FFTW REQUIRED) # Transforms

elements_add_library(Linx src/lib/*.cpp
                     INCLUDE_DIRS Boost Cfitsio Eigen3 FFTW
                     LINK_LIBRARIES Boost Cfitsio Eigen3 FFTW
                     PUBLIC_HEADERS Linx)
