CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

elements_subdir(RasterPrograms)

elements_depends_on_subdirs(RasterFilter)

elements_add_library(RasterPrograms src/lib/*.cpp
                     INCLUDE_DIRS RasterFilter
                     LINK_LIBRARIES RasterFilter
                     PUBLIC_HEADERS RasterPrograms)

elements_add_executable(RasterBenchmarkCorrelation src/program/RasterBenchmarkCorrelation.cpp
                     INCLUDE_DIRS RasterFilter RasterPrograms
                     LINK_LIBRARIES RasterFilter RasterPrograms)

elements_add_unit_test(ProgramOptions tests/src/ProgramOptions_test.cpp 
                     EXECUTABLE RasterPrograms_ProgramOptions_test
                     LINK_LIBRARIES RasterPrograms
                     TYPE Boost)

elements_install_conf_files()
