CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

find_package(ElementsProject)

set(USE_SPHINX OFF
    CACHE STRING "Sphinx API documentation" 
    FORCE)

elements_project(
    CnesRaster 1.0 # Cnes.Raster is forbidden because of dot
    USE Elements 6.0.1
    DESCRIPTION "Multidimensional raster image processing")
