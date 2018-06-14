cmake_minimum_required(VERSION 3.5.1)

project(nogdb VERSION 0.11.0 LANGUAGES C CXX)

file(GLOB nogdb_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB nogdb_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp)
file(GLOB nogdb_PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
file(GLOB nogdb_TEST ${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp)
file(GLOB nogdb_TEST_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/test/*.h)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED OFF)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(COMPILE_OPTIONS
    -std=c++11
    -Os
    -Ofast
    -W
    -Wall
    -Wno-unused-parameter
    -Wno-unused-variable
    -Wno-unknown-pragmas
    -Wno-format-extra-args
    -fPIC
    -march=native
)

## TARGET lmdb
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/lmdb)

## TARGET lemon++
set(lemon_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/lemonxx)
set(lemon_SOURCE ${lemon_DIR}/lemon.c)
set_source_files_properties(${lemon_SOURCE} PROPERTIES COMPILE_FLAGS -w)
add_executable(lemon++ EXCLUDE_FROM_ALL ${lemon_SOURCE})
target_compile_definitions(lemon++ PUBLIC LEMONPLUSPLUS=1 TEMPLATE_EXTENSION=\".cxx\")

## TARGET sql_parser
set(sql_parser_CPP ${CMAKE_CURRENT_SOURCE_DIR}/src/sql_parser.cpp)
set(sql_parser_Y ${CMAKE_CURRENT_SOURCE_DIR}/src/sql_parser.y)
set_source_files_properties(${sql_parser_CPP} PROPERTIES COMPILE_FLAGS -w)
add_custom_target(sql_parser.cpp
    DEPENDS lemon++ ${sql_parser_CPP} ${sql_parser_Y}
)
add_custom_command(
    OUTPUT ${sql_parser_CPP}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src
    COMMAND lemon++ -q -T${lemon_DIR}/lempar.cxx ${sql_parser_Y}
    MAIN_DEPENDENCY ${sql_parser_Y}
)

## TARGET nogdb
add_library(nogdb
    ${nogdb_SOURCE}
    ${nogdb_HEADER}
    ${nogdb_PUBLIC}
    $<TARGET_OBJECTS:lmdb_OBJ>
    ${sql_parser_CPP}
    src/lmdb_interface.cpp
)
add_dependencies(nogdb lmdb_OBJ sql_parser.cpp)
target_include_directories(nogdb
    PRIVATE
        /usr/local/include
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/
        ${CMAKE_CURRENT_SOURCE_DIR}/include/
)
target_compile_options(nogdb
    PUBLIC
        -std=c++11
    PRIVATE
        ${COMPILE_OPTIONS}
)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(nogdb Threads::Threads)

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

## TARGET install
install(TARGETS nogdb DESTINATION lib)
install(DIRECTORY include/ DESTINATION include/nogdb
    FILES_MATCHING PATTERN "*.h"
)
install(DIRECTORY lib/lmdb/ DESTINATION include/nogdb/lmdb
    FILES_MATCHING PATTERN "lib/lmdb/*.h"
)
install(DIRECTORY lib/boost/ DESTINATION include/nogdb/boost
    FILES_MATCHING PATTERN "lib/boost/*.hpp"
)
