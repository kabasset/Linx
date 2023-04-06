CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

elements_subdir(LinxRun)

elements_depends_on_subdirs(ElementsKernel)
elements_depends_on_subdirs(LinxTransforms)

elements_add_library(LinxRun src/lib/*.cpp
                     INCLUDE_DIRS LinxTransforms ElementsKernel
                     LINK_LIBRARIES LinxTransforms ElementsKernel
                     PUBLIC_HEADERS LinxRun)

elements_add_executable(LinxBenchmarkBuffers src/program/LinxBenchmarkBuffers.cpp
                     INCLUDE_DIRS LinxRun
                     LINK_LIBRARIES LinxRun)
elements_add_executable(LinxBenchmarkCorrelation src/program/LinxBenchmarkCorrelation.cpp
                     INCLUDE_DIRS LinxRun
                     LINK_LIBRARIES LinxRun)
elements_add_executable(LinxBenchmarkExp src/program/LinxBenchmarkExp.cpp
                     INCLUDE_DIRS LinxRun
                     LINK_LIBRARIES LinxRun)
elements_add_executable(LinxBenchmarkIteration src/program/LinxBenchmarkIteration.cpp
                     INCLUDE_DIRS LinxRun
                     LINK_LIBRARIES LinxRun)
elements_add_executable(LinxBenchmarkRegions src/program/LinxBenchmarkRegions.cpp
                     INCLUDE_DIRS LinxRun
                     LINK_LIBRARIES LinxRun)

elements_add_unit_test(Chronometer tests/src/Chronometer_test.cpp 
                     EXECUTABLE LinxRun_Chronometer_test
                     LINK_LIBRARIES LinxRun
                     TYPE Boost)
elements_add_unit_test(IterationBenchmark tests/src/IterationBenchmark_test.cpp 
                     EXECUTABLE LinxRun_IterationBenchmark_test
                     LINK_LIBRARIES LinxRun
                     TYPE Boost)
elements_add_unit_test(ProgramOptions tests/src/ProgramOptions_test.cpp 
                     EXECUTABLE LinxRun_ProgramOptions_test
                     LINK_LIBRARIES LinxRun
                     TYPE Boost)
elements_add_unit_test(StepperPipeline tests/src/StepperPipeline_test.cpp 
                     EXECUTABLE LinxRun_StepperPipeline_test
                     LINK_LIBRARIES LinxRun
                     TYPE Boost)
