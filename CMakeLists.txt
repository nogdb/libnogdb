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

option(nogdb_BuildTests "Build the tests when enabled." ON)

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
)
add_dependencies(nogdb lmdb_OBJ sql_parser.cpp)
target_include_directories(nogdb
    PRIVATE
        /usr/local/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include/nogdb/
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
set(TEST_COMPILE_OPTIONS
    -std=c++11
    -W
    -Wall
    -Wno-unused-parameter
    -Wno-unused-variable
    -Wno-unused-but-set-variable
    -Wno-unknown-warning-option
)

set(RUNTEST_PATH "${CMAKE_CURRENT_SOURCE_DIR}/test/runtest.cpp")
string(REPLACE ${RUNTEST_PATH} "" nogdb_TEST "${nogdb_TEST}")
add_library(nogdb_test_object EXCLUDE_FROM_ALL OBJECT ${nogdb_TEST} ${nogdb_TEST_HEADER})
target_include_directories(nogdb_test_object PRIVATE $<TARGET_PROPERTY:nogdb,INTERFACE_INCLUDE_DIRECTORIES>)

# runtest_executable(name)
function(runtest_executable name)
    string(TOUPPER ${name} upper_name)
    add_executable(runtest_${name} EXCLUDE_FROM_ALL ${RUNTEST_PATH} $<TARGET_OBJECTS:nogdb_test_object>)
    target_link_libraries(runtest_${name} nogdb)
    target_compile_options(runtest_${name}
        PRIVATE
            ${TEST_COMPILE_OPTIONS}
            -DTEST_${upper_name}_OPERATIONS
    )
    add_test(build_runtest_${name} "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --target runtest_${name})
    add_test(NAME test_${name} COMMAND runtest_${name})
    set_tests_properties(test_${name} PROPERTIES DEPENDS ctest_build_test_${name})
endfunction()

if(nogdb_BuildTests)
    enable_testing()
    runtest_executable(context)
    runtest_executable(schema)
    runtest_executable(record)
    runtest_executable(misc)
    runtest_executable(graph)
    runtest_executable(find)
    runtest_executable(inheritance)
    runtest_executable(index)
    runtest_executable(schema_txn)
    runtest_executable(txn)
    runtest_executable(sql)
endif()

## TARGET install
install(TARGETS nogdb DESTINATION lib)
install(DIRECTORY include/ DESTINATION include
)
install(DIRECTORY lib/lmdb/ DESTINATION include/nogdb/lmdb
    FILES_MATCHING PATTERN "lib/lmdb/*.h"
)
