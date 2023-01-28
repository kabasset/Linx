CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

elements_subdir(LitlRun)

elements_depends_on_subdirs(ElementsKernel)
elements_depends_on_subdirs(LitlTransforms)

elements_add_library(LitlRun src/lib/*.cpp
                     INCLUDE_DIRS LitlTransforms ElementsKernel
                     LINK_LIBRARIES LitlTransforms ElementsKernel
                     PUBLIC_HEADERS LitlRun)

elements_add_executable(LitlBenchmarkBuffers src/program/LitlBenchmarkBuffers.cpp
                     INCLUDE_DIRS LitlRun
                     LINK_LIBRARIES LitlRun)
elements_add_executable(LitlBenchmarkCorrelation src/program/LitlBenchmarkCorrelation.cpp
                     INCLUDE_DIRS LitlRun
                     LINK_LIBRARIES LitlRun)
elements_add_executable(LitlBenchmarkExp src/program/LitlBenchmarkExp.cpp
                     INCLUDE_DIRS LitlRun
                     LINK_LIBRARIES LitlRun)
elements_add_executable(LitlBenchmarkIteration src/program/LitlBenchmarkIteration.cpp
                     INCLUDE_DIRS LitlRun
                     LINK_LIBRARIES LitlRun)
elements_add_executable(LitlBenchmarkRegions src/program/LitlBenchmarkRegions.cpp
                     INCLUDE_DIRS LitlRun
                     LINK_LIBRARIES LitlRun)

elements_add_unit_test(Chronometer tests/src/Chronometer_test.cpp 
                     EXECUTABLE LitlRun_Chronometer_test
                     LINK_LIBRARIES LitlRun
                     TYPE Boost)
elements_add_unit_test(IterationBenchmark tests/src/IterationBenchmark_test.cpp 
                     EXECUTABLE LitlRun_IterationBenchmark_test
                     LINK_LIBRARIES LitlRun
                     TYPE Boost)
elements_add_unit_test(ProgramOptions tests/src/ProgramOptions_test.cpp 
                     EXECUTABLE LitlRun_ProgramOptions_test
                     LINK_LIBRARIES LitlRun
                     TYPE Boost)
elements_add_unit_test(StepperPipeline tests/src/StepperPipeline_test.cpp 
                     EXECUTABLE LitlRun_StepperPipeline_test
                     LINK_LIBRARIES LitlRun
                     TYPE Boost)

elements_install_conf_files()
