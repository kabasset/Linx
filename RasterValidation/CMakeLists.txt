CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

elements_subdir(RasterValidation)

elements_depends_on_subdirs(RasterFilter)
elements_depends_on_subdirs(ElementsKernel)
elements_add_library(RasterValidation src/lib/*.cpp
                     INCLUDE_DIRS RasterFilter ElementsKernel
                     LINK_LIBRARIES RasterFilter ElementsKernel
                     PUBLIC_HEADERS RasterValidation)
elements_add_executable(RasterBenchmarkCorrelation src/program/RasterBenchmarkCorrelation.cpp
                     INCLUDE_DIRS RasterFilter RasterValidation
                     LINK_LIBRARIES RasterFilter RasterValidation)
elements_add_executable(RasterBenchmarkIteration src/program/RasterBenchmarkIteration.cpp
                     INCLUDE_DIRS ElementsKernel RasterValidation
                     LINK_LIBRARIES ElementsKernel RasterValidation)
elements_add_unit_test(ProgramOptions tests/src/ProgramOptions_test.cpp 
                     EXECUTABLE RasterValidation_ProgramOptions_test
                     LINK_LIBRARIES RasterValidation
                     TYPE Boost)
elements_add_unit_test(IterationBenchmark tests/src/IterationBenchmark_test.cpp 
                     EXECUTABLE RasterValidation_IterationBenchmark_test
                     LINK_LIBRARIES RasterValidation
                     TYPE Boost)
elements_add_unit_test(Chronometer tests/src/Chronometer_test.cpp 
                     EXECUTABLE RasterValidation_Chronometer_test
                     LINK_LIBRARIES RasterValidation
                     TYPE Boost)

elements_install_conf_files()
