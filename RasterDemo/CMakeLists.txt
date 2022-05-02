CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

elements_subdir(RasterDemo)

elements_depends_on_subdirs(Raster)

elements_add_executable(RasterDemoBasics src/program/RasterDemoBasics.cpp
                     INCLUDE_DIRS ElementsKernel
                     LINK_LIBRARIES ElementsKernel)

elements_install_conf_files()
