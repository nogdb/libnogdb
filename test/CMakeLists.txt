## TARGET test
enable_testing()

set(TEST_COMPILE_OPTIONS
        -std=c++11
        -W
        -Wall
        -Wno-unused-parameter
        -Wno-unused-variable
        )

add_executable(runtest_context EXCLUDE_FROM_ALL ${nogdb_TEST} ${nogdb_TEST_HEADER})
target_link_libraries(runtest_context nogdb)
target_compile_options(runtest_context
        PRIVATE
        ${TEST_COMPILE_OPTIONS}
        -Wno-unused-but-set-variable
        -Wno-unknown-warning-option
        -DTEST_CONTEXT_OPERATIONS
        )
add_test(build_runtest_context "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --target runtest_context)
add_test(NAME test_context COMMAND runtest_context)
set_tests_properties(test_context PROPERTIES DEPENDS ctest_build_test_context)

add_executable(runtest_schema EXCLUDE_FROM_ALL ${nogdb_TEST} ${nogdb_TEST_HEADER})
target_link_libraries(runtest_schema nogdb)
target_compile_options(runtest_schema
        PRIVATE
        ${TEST_COMPILE_OPTIONS}
        -Wno-unused-but-set-variable
        -Wno-unknown-warning-option
        -DTEST_SCHEMA_OPERATIONS
        )
add_test(build_runtest_schema "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --target runtest_schema)
add_test(NAME test_schema COMMAND runtest_schema)
set_tests_properties(test_schema PROPERTIES DEPENDS ctest_build_test_schema)

add_executable(runtest_record EXCLUDE_FROM_ALL ${nogdb_TEST} ${nogdb_TEST_HEADER})
target_link_libraries(runtest_record nogdb)
target_compile_options(runtest_record
        PRIVATE
        ${TEST_COMPILE_OPTIONS}
        -Wno-unused-but-set-variable
        -Wno-unknown-warning-option
        -DTEST_RECORD_OPERATIONS
        )
add_test(build_runtest_record "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --target runtest_record)
add_test(NAME test_record COMMAND runtest_record)
set_tests_properties(test_record PROPERTIES DEPENDS ctest_build_test_record)

add_executable(runtest_misc EXCLUDE_FROM_ALL ${nogdb_TEST} ${nogdb_TEST_HEADER})
target_link_libraries(runtest_misc nogdb)
target_compile_options(runtest_misc
        PRIVATE
        ${TEST_COMPILE_OPTIONS}
        -Wno-unused-but-set-variable
        -Wno-unknown-warning-option
        -DTEST_MISC_OPERATIONS
        )
add_test(build_runtest_misc "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --target runtest_misc)
add_test(NAME test_misc COMMAND runtest_misc)
set_tests_properties(test_misc PROPERTIES DEPENDS ctest_build_test_misc)

add_executable(runtest_graph EXCLUDE_FROM_ALL ${nogdb_TEST} ${nogdb_TEST_HEADER})
target_link_libraries(runtest_graph nogdb)
target_compile_options(runtest_graph
        PRIVATE
        ${TEST_COMPILE_OPTIONS}
        -Wno-unused-but-set-variable
        -Wno-unknown-warning-option
        -DTEST_GRAPH_OPERATIONS
        )
add_test(build_runtest_graph "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --target runtest_graph)
add_test(NAME test_graph COMMAND runtest_graph)
set_tests_properties(test_graph PROPERTIES DEPENDS ctest_build_test_graph)

add_executable(runtest_find EXCLUDE_FROM_ALL ${nogdb_TEST} ${nogdb_TEST_HEADER})
target_link_libraries(runtest_find nogdb)
target_compile_options(runtest_find
        PRIVATE
        ${TEST_COMPILE_OPTIONS}
        -Wno-unused-but-set-variable
        -Wno-unknown-warning-option
        -DTEST_FIND_OPERATIONS
        )
add_test(build_runtest_find "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --target runtest_find)
add_test(NAME test_find COMMAND runtest_find)
set_tests_properties(test_find PROPERTIES DEPENDS ctest_build_test_find)

add_executable(runtest_inheritance EXCLUDE_FROM_ALL ${nogdb_TEST} ${nogdb_TEST_HEADER})
target_link_libraries(runtest_inheritance nogdb)
target_compile_options(runtest_inheritance
        PRIVATE
        ${TEST_COMPILE_OPTIONS}
        -Wno-unused-but-set-variable
        -Wno-unknown-warning-option
        -DTEST_INHERITANCE_OPERATIONS
        )
add_test(build_runtest_inheritance "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --target runtest_inheritance)
add_test(NAME test_inheritance COMMAND runtest_inheritance)
set_tests_properties(test_inheritance PROPERTIES DEPENDS ctest_build_test_inheritance)

add_executable(runtest_index EXCLUDE_FROM_ALL ${nogdb_TEST} ${nogdb_TEST_HEADER})
target_link_libraries(runtest_index nogdb)
target_compile_options(runtest_index
        PRIVATE
        ${TEST_COMPILE_OPTIONS}
        -Wno-unused-but-set-variable
        -Wno-unknown-warning-option
        -DTEST_INDEX_OPERATIONS
        )
add_test(build_runtest_index "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --target runtest_index)
add_test(NAME test_index COMMAND runtest_index)
set_tests_properties(test_index PROPERTIES DEPENDS ctest_build_test_index)

add_executable(runtest_schema_txn EXCLUDE_FROM_ALL ${nogdb_TEST} ${nogdb_TEST_HEADER})
target_link_libraries(runtest_schema_txn nogdb)
target_compile_options(runtest_schema_txn
        PRIVATE
        ${TEST_COMPILE_OPTIONS}
        -Wno-unused-but-set-variable
        -Wno-unknown-warning-option
        -DTEST_SCHEMA_TXN_OPERATIONS
        )
add_test(build_runtest_schema_txn "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --target runtest_schema_txn)
add_test(NAME test_schema_txn COMMAND runtest_schema_txn)
set_tests_properties(test_schema_txn PROPERTIES DEPENDS ctest_build_test_schema_txn)

add_executable(runtest_txn EXCLUDE_FROM_ALL ${nogdb_TEST} ${nogdb_TEST_HEADER})
target_link_libraries(runtest_txn nogdb)
target_compile_options(runtest_txn
        PRIVATE
        ${TEST_COMPILE_OPTIONS}
        -Wno-unused-but-set-variable
        -Wno-unknown-warning-option
        -DTEST_TXN_OPERATIONS
        )
add_test(build_runtest_txn "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --target runtest_txn)
add_test(NAME test_txn COMMAND runtest_txn)
set_tests_properties(test_txn PROPERTIES DEPENDS ctest_build_test_txn)

add_executable(runtest_sql EXCLUDE_FROM_ALL ${nogdb_TEST} ${nogdb_TEST_HEADER})
target_link_libraries(runtest_sql nogdb)
target_compile_options(runtest_sql
        PRIVATE
        ${TEST_COMPILE_OPTIONS}
        -DTEST_SQL_OPERATIONS
        )
add_test(build_runtest_sql "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --target runtest_sql)
add_test(NAME test_sql COMMAND runtest_sql)
set_tests_properties(test_sql PROPERTIES DEPENDS ctest_build_test_sql)