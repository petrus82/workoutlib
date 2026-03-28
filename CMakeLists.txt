cmake_minimum_required(VERSION 4.3.0)

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

set(LIBRARY_TYPE SHARED)

file (GLOB MODULE_FILES
  workout.cppm
  planworkout.cppm
  fitworkout.cppm
  ergworkout.cppm
  mrcworkout.cppm
  fit_profile.cppm
  fit.cppm
  "${CMAKE_CURRENT_BINARY_DIR}/config.cppm"
)

file (GLOB SOURCE_FILES
  workout.cpp
  planworkout.cpp
  fitworkout.cpp
  ergworkout.cpp
  mrcworkout.cpp
)

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
)

target_sources(${PROJECT_NAME}
PRIVATE
  ${SOURCE_FILES}

PUBLIC
  FILE_SET CXX_MODULES
  BASE_DIRS ${BASE_DIRS}
  FILES ${MODULE_FILES}
)
target_include_directories(${PROJECT_NAME} PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)
target_link_libraries(${PROJECT_NAME} PUBLIC
  Garmin::GarminFit
)

install(TARGETS ${LIBRARY_NAME} 
  DESTINATION ${CMAKE_BINARY_DIR}/build
  COMPONENT lib
)

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

if(TESTING)
# Testing
include(CTest)
enable_testing()

file (GLOB TEST_FILES
  testing/workout_test.cpp
)

add_executable(Tests
  ${TEST_FILES}
  ${SOURCE_FILES}
)
target_sources(Tests
  PRIVATE
    ${TEST_FILES}
  PRIVATE
    FILE_SET CXX_MODULES
    BASE_DIRS ${BASE_DIRS}
    FILES ${MODULE_FILES}
)

target_link_libraries(Tests PUBLIC
  Garmin::GarminFit
  gtest
)

# Copy .fit test files into build
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/testfiles/" DESTINATION ${CMAKE_BINARY_DIR})

add_test(
  NAME WorkoutTests
  COMMAND Tests
)
endif()