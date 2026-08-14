
find_program(LLVM_PROFDATA_EXECUTABLE llvm-profdata REQUIRED)
find_program(LLVM_COV_EXECUTABLE llvm-cov REQUIRED)

set(COVERAGE_RAW_FILE   "default.profraw")
set(COVERAGE_DATA_FILE  "default.profdata")
set(COVERAGE_OUT  "lcov.info")

file(GENERATE
    OUTPUT "coverage.sh"
    CONTENT 
        "
        cd ${CMAKE_BINARY_DIR}
        echo \"PWD: \$\{PWD\}\"
        echo \"Generating Raw LLVM profile data. Make sure the Test target was generated before calling this.\"
        ${CMAKE_COMMAND} -E env LLVM_PROFILE_FILE=${COVERAGE_RAW_FILE} $<TARGET_FILE:Tests>

        echo \"Merging LLVM profile data.\"
        ${LLVM_PROFDATA_EXECUTABLE} merge --sparse ${COVERAGE_RAW_FILE} -o ${COVERAGE_DATA_FILE}

        echo \"Generating LCOV report.\"
        ${LLVM_COV_EXECUTABLE} export --format=lcov --object=$<TARGET_FILE:Tests> --instr-profile=${COVERAGE_DATA_FILE} > ${COVERAGE_OUT}
        "
)

add_custom_command(
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    OUTPUT "${CMAKE_BINARY_DIR}/lcov.info"
    COMMENT "Generating coverage data by calling ${CMAKE_BINARY_DIR}/coverage.sh"
    COMMAND "/usr/bin/bash" "${CMAKE_BINARY_DIR}/coverage.sh"
)

add_custom_target(coverage_target
    DEPENDS "${COVERAGE_OUT}"
)
