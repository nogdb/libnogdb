## TARGET test/module
enable_testing()

set(TEST_COMPILE_OPTIONS
        -std=c++11
        -W
        -Wall
        -Wno-unused-parameter
        -Wno-unused-variable
        )

add_executable(gtest_module EXCLUDE_FROM_ALL ${nogdb_TEST_MODULE} ${nogdb_TEST_MODULE_HEADER})
target_link_libraries(gtest_module nogdb)
target_compile_options(gtest_module
        PRIVATE
        ${TEST_COMPILE_OPTIONS}
        -Wno-unused-but-set-variable
        -Wno-unknown-warning-option
        )
add_test(build_gtest_module "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --target gtest_module)
add_test(NAME unittest_module COMMAND gtest_module)
set_tests_properties(unittest_module PROPERTIES DEPENDS ctest_build_gtest_module)
