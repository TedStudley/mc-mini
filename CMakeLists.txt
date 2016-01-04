cmake_minimum_required(VERSION 2.8)

set(PROJECT_NAME mc-mini)
set(PROJECT_VERSION 1.1.0)

# https://cmake.org/cmake/help/v3.0/policy/CMP0048.html#policy:CMP0048
# Attempt to set the project version field, if that's supported. CMake versions
# after 3.0 should support policy CMP0048.
if(POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW)
  project(${PROJECT_NAME} VERSION ${PROJECT_VERSION})
else()
  project(${PROJECT_NAME})
endif()

# Include additional required CMake modules.
set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    ${CMAKE_SOURCE_DIR}/cmake)

# //=========================\\
# || CMake Compile Arguments ||
# \\=========================//
# Enable debug information by default.
option(DEBUG_ENABLED "Enable debugging info" ON)
# Enable the test make target by default.
option(TESTS_ENABLED "Enable automatic tests" ON)
# Disable testing coverage by default.
option(COVERAGE_ENABLED "Enable test coverage" OFF)


# //================\\
# || CMake Settings ||
# \\================//
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

SET(LIBRARY_OUTPUT_PATH
    ${CMAKE_BINARY_DIR}/lib)
SET(EXECUTABLE_OUTPUT_PATH
    ${CMAKE_BINARY_DIR})

if(NOT CMAKE_BUILD_TYPE)
  message(STATUS "No build type selected, defaulting to Debug")
  set(CMAKE_BUILD_TYPE "Debug")
endif()

include_directories(${CMAKE_SOURCE_DIR}/include)

# //======================\\
# || C/CXX Compiler Flags ||
# \\======================//
include(CheckCXXCompilerFlag)

# Use more warnings by default
set(CMAKE_CXX_BASE_FLAGS "-Wall")
# Use more warnings by default
set(CMAKE_CXX_FLAGS "-Wall")

# Check if the compiler supports at least -std=c++0x
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
if(COMPILER_SUPPORTS_CXX11)
  # Use -std=c++11 if it's supported
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
  # Otherwise, fall back to -std=c++0x
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
  # Otherwise, fail to compile.
  message(FATAL_ERROR "Compiler does not support C++11")
endif()

if(DEBUG_ENABLED)
  # Disable optimization and enable debugging information if debugging is
  # enabled.
  set(CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS} -O0 -g")
endif()

if(COVERAGE_ENABLED)
  # Set extra coverage-related flags if coverage is enabled.
  set(CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS} -fPIC -fprofile-arcs -ftest-coverage")
endif()

# //==========================\\
# || Documentation Generation ||
# \\==========================//
# Check if doxygen is present on the system
find_package(Doxygen)
if(DOXYGEN_FOUND)
  # If doxygen is present, then add a custom target to generate documentation
  # for the codebase.
  configure_file(${CMAKE_SOURCE_DIR}/docs/Doxyfile.in ${CMAKE_SOURCE_DIR}/docs/Doxyfile @ONLY)
  add_custom_target(docs
    ${DOXYGEN_EXECUTABLE} ${CMAKE_SOURCE_DIR}/docs/Doxyfile
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/docs
    COMMENT "Generating API documentation with Doxygen" VERBATIM
  )
endif(DOXYGEN_FOUND)


# //==============================\\
# || Additional Required Packages ||
# \\==============================//
# Boost, a collection of useful C++ utilities and idioms
find_package(Boost REQUIRED COMPONENTS
    filesystem
    system)
include_directories(${Boost_INCLUDE_DIR})
set(LIBRARIES ${LIBRARIES} ${Boost_LIBRARIES})

# Eigen, a powerful linear algebra header-only library
# Set EIGEN3_INCLUDE_DIR if it's set in ENV. This way TravisCI can use a
# local copy of Eigen instead of the outdated version on apt.
if(DEFINED ENV{EIGEN3_INCLUDE_DIR})
  set(EIGEN3_INCLUDE_DIR $ENV{EIGEN3_INCLUDE_DIR})
  message(STATUS "Using local Eigen3 headers")
endif()
find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})

# HDF5, an output library
find_package(HDF5 REQUIRED)
include_directories(${HDF5_INCLUDE_DIR})
set(LIBRARIES ${LIBRARIES} ${HDF5_LIBRARIES})

# //======================\\
# || CMake Subdirectories ||
# \\======================//
# Include basic library and executable targets
add_subdirectory(source)

# Include testing and coverage targets
if(TESTS_ENABLED)
  add_subdirectory(test)
endif()

# vim:ft=cmake
