CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

elements_subdir(LinxRun)

elements_depends_on_subdirs(Linx)

find_package(Boost) # test

elements_add_library(LinxRun src/lib/*.cpp
                     INCLUDE_DIRS Linx
                     LINK_LIBRARIES Linx
                     PUBLIC_HEADERS LinxRun)

elements_add_executable(LinxBenchmarkBuffers src/program/LinxBenchmarkBuffers.cpp
                     INCLUDE_DIRS LinxRun
                     LINK_LIBRARIES LinxRun)
elements_add_executable(LinxBenchmarkConvolution src/program/LinxBenchmarkConvolution.cpp
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
elements_add_executable(LinxDemoAffinity src/program/LinxDemoAffinity.cpp
                    INCLUDE_DIRS LinxRun
                    LINK_LIBRARIES LinxRun)
elements_add_executable(LinxMaskCosmics src/program/LinxMaskCosmics.cpp
                    INCLUDE_DIRS LinxRun
                    LINK_LIBRARIES LinxRun)

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
elements_add_unit_test(Timer tests/src/Timer_test.cpp 
                     EXECUTABLE LinxRun_Timer_test
                     LINK_LIBRARIES LinxRun
                     TYPE Boost)

elements_install_python_modules()

elements_add_python_program(LinxBenchmarkPythonConvolution LinxRun.LinxBenchmarkPythonConvolution)
