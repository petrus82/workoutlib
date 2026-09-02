cmake_minimum_required(VERSION 4.4.0)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/CMake")

include (STDModules)

project(workoutlib 
  VERSION 0.0.1
  DESCRIPTION "C++ Library for opening and saving fit, erg, mrc and plan files."
  LANGUAGES CXX
  HOMEPAGE_URL https://github.com/petrus82/WorkoutEditor
)

# Configuration switches
set(TESTING ON)
message("Testing: ${TESTING}")

find_package(GarminFit REQUIRED)

find_library(libmd_LIBRARY NAMES libmd.so PATHS /usr/lib/)
message ("Found libmd at ${libmd_LIBRARY}.")

set(LIBRARY_TYPE SHARED)

# Generate config.cppm
file(GENERATE
    OUTPUT "config.cppm"
    CONTENT 
        "
        export module config;
        import std;
        export constexpr const bool TESTING = $<IF:$<STREQUAL:${TESTING},ON>,true,false>;
        "
)

file (GLOB MODULE_FILES
  common_declarations.cppm  
  filehandling.cppm
  fileconcept.cppm
  textfiles.cppm
  planfiles.cppm
  fitfiles.cppm
  sha256.cppm
  locale.cppm
  intensity.cppm
  interval.cppm
  workout.cppm
  fit_profile.cppm
  fit.cppm
)
# config.cppm is emitted by file(GENERATE) during the generate phase, so it
# does not exist on disk yet when the GLOB runs on a fresh build tree.
list(APPEND MODULE_FILES "${CMAKE_CURRENT_BINARY_DIR}/config.cppm")
message("Module files: ${MODULE_FILES}")

set(BASE_DIRS
  ${CMAKE_CURRENT_SOURCE_DIR}
)

message("Base dirs: ${BASE_DIRS}")

# LTO
include(CheckIPOSupported)
check_ipo_supported(RESULT LTO_SUPPORTED OUTPUT error)
if (LTO_SUPPORTED AND CMAKE_BUILD_TYPE MATCHES Release)
    message(STATUS "LTO enabled")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
endif()

add_library(${PROJECT_NAME} ${LIBRARY_TYPE} 
  ${SOURCE_FILES}
)

target_sources(${PROJECT_NAME}

PUBLIC
  FILE_SET CXX_MODULES
  BASE_DIRS ${BASE_DIRS}
  FILES ${MODULE_FILES}
)

message("Include dirs: ${CMAKE_CURRENT_SOURCE_DIR}, ${CMAKE_CURRENT_BINARY_DIR}")

target_include_directories(${PROJECT_NAME} PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)
target_link_libraries(${PROJECT_NAME} PUBLIC
  Garmin::GarminFit
)

install(TARGETS ${PROJECT_NAME}
  DESTINATION ${CMAKE_BINARY_DIR}/build
  COMPONENT lib
)

if(TESTING)
# Testing


file (GLOB TEST_FILES   
  testing/file_tests.cpp
  testing/textfile_tests.cpp
  testing/planfile_tests.cpp
  testing/intensity_tests.cpp
  testing/interval_tests.cpp
)

add_executable(Tests
  ${TEST_FILES}
)
target_sources(Tests
  PUBLIC
    ${TEST_FILES}
  PUBLIC
    FILE_SET CXX_MODULES
    BASE_DIRS ${BASE_DIRS}
    FILES ${MODULE_FILES}
)

target_link_libraries(Tests PUBLIC
  Garmin::GarminFit
  gtest
  ${libmd_LIBRARY}
)
include(CTest)
enable_testing()

# Enable test coverage data
target_compile_options(Tests PRIVATE -fprofile-instr-generate -fcoverage-mapping)
target_link_options(Tests PRIVATE -fprofile-instr-generate -fcoverage-mapping)

# Copy .fit test files into build
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/testfiles/" DESTINATION ${CMAKE_BINARY_DIR})

add_test(
  NAME WorkoutTests
  COMMAND Tests
  DEPENDS coverage_target
)
# Get test coverage data
include(Coverage)

endif()