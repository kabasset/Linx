# Setup

cmake_minimum_required(VERSION 3.16)
project(
  Linx
  VERSION 1.0
  LANGUAGES CXX
  DESCRIPTION "Extensible, cross-platform ND image laboratory")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# Dependencies

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")
find_package(Boost COMPONENTS unit_test_framework program_options REQUIRED)
find_package(Kokkos 4.4.01 REQUIRED CONFIG) # SequentialHostInit >= 4.4.01
find_package(Cfitsio REQUIRED) # FIXME make optional

# Library

add_library(Linx INTERFACE)
target_include_directories(Linx INTERFACE include)
target_link_libraries(Linx INTERFACE ${Boost_LIBRARIES} Kokkos::kokkos ${CFITSIO_LIBRARY}) #FIXME make CFITSIO optional

# Tests

enable_testing()
add_subdirectory(tests)
add_subdirectory(examples)

# Doc

find_package(Doxygen)

if(DOXYGEN_FOUND)
  set(DOXYGEN_INPUT_DIR ${PROJECT_SOURCE_DIR}/include)
  set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/doc/doxygen)
  set(DOXYGEN_INDEX_FILE ${DOXYGEN_OUTPUT_DIR}/xml/index.xml)
  set(DOXYFILE_IN ${PROJECT_SOURCE_DIR}/cmake/doc/Doxyfile.in)
  set(DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

  configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY) # Interpolate Doxyfile
  file(MAKE_DIRECTORY ${DOXYGEN_OUTPUT_DIR})

  add_custom_target(doc
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM)
endif(DOXYGEN_FOUND)

# Executables

add_executable(LinCosmicX src/LinCosmicX.cpp)
target_link_libraries(LinCosmicX Linx)

add_executable(LinxBenchmarkFilters src/LinxBenchmarkFilters.cpp)
target_link_libraries(LinxBenchmarkFilters Linx)

add_executable(KokkosBenchmarkConvolution src/KokkosBenchmarkConvolution.cpp)
target_link_libraries(KokkosBenchmarkConvolution Linx)

add_executable(KokkosBenchmarkExp src/KokkosBenchmarkExp.cpp)
target_link_libraries(KokkosBenchmarkExp Linx)

add_executable(KokkosBenchmarkIteration src/KokkosBenchmarkIteration.cpp)
target_link_libraries(KokkosBenchmarkIteration Linx)
