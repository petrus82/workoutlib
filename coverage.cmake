
find_program(LLVM_PROFDATA_EXECUTABLE llvm-profdata REQUIRED)

set(COVERAGE_RAW_FILE   "default.profraw")
set(COVERAGE_DATA_FILE  "default.profdata")

# Run tests with LLVM_PROFILE_FILE set so instrumentation writes profraw data.
add_custom_command(
    OUTPUT "${COVERAGE_RAW_FILE}"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    COMMENT "Generating Raw LLVM profile data"
    COMMAND ${CMAKE_COMMAND} -E env LLVM_PROFILE_FILE=${COVERAGE_RAW_FILE} $<TARGET_FILE:Tests>
    VERBATIM
)

# Merge the raw profile into profdata.
add_custom_command(
    OUTPUT "${COVERAGE_DATA_FILE}"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    COMMENT "Merging LLVM profile data"
    DEPENDS "${COVERAGE_RAW_FILE}"
    COMMAND "${LLVM_PROFDATA_EXECUTABLE}" merge --sparse
            "${COVERAGE_RAW_FILE}"
            -o "${COVERAGE_DATA_FILE}"
    VERBATIM
)

add_custom_target(coverage_target
    DEPENDS "${COVERAGE_DATA_FILE}"
)
