cmake_minimum_required(VERSION 3.5.1)

project(nogdb VERSION 1.2.0 LANGUAGES C CXX)

file(GLOB nogdb_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB nogdb_SOURCE_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp)
file(GLOB nogdb_PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/nogdb/*.h)
file(GLOB nogdb_API_TEST ${CMAKE_CURRENT_SOURCE_DIR}/test/func_test/*.cpp)
file(GLOB nogdb_API_TEST_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/test/func_test/*.h)
file(GLOB nogdb_UNIT_TEST ${CMAKE_CURRENT_SOURCE_DIR}/test/unit_test/*.cpp)
file(GLOB nogdb_UNIT_TEST_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/test/unit_test/*.h)
file(GLOB nogdb_UNIT_TEST_LMDB ${CMAKE_CURRENT_SOURCE_DIR}/test/unit_test/lmdb_engine/*.cpp)
file(GLOB nogdb_UNIT_TEST_LMDB_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/test/unit_test/lmdb_engine/*.h)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED OFF)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(COMPILE_OPTIONS
    -std=c++11
    -W
    -Wall
    -Wno-unused-parameter
    -Wno-unused-variable
    -Wno-unknown-pragmas
    -Wno-format-extra-args
    -Wno-c++1z-extensions
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
    ${nogdb_SOURCE_HEADER}
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
if(NOT APPLE)
    target_link_libraries(nogdb atomic)
endif()

## TARGET unit_test
set(UNIT_TEST_COMPILE_OPTIONS
    -std=c++11
    -W
    -Wall
    -Wno-unused-parameter
    -Wno-unused-variable
    -Wno-unknown-pragmas
    -Wno-format-extra-args
    -fPIC
    -march=native
)

## TARGET func_test
set(FUNC_TEST_COMPILE_OPTIONS
    -std=c++11
    -W
    -Wall
    -Wno-unused-parameter
    -Wno-unused-variable
    -Wno-unused-but-set-variable
    -Wno-unknown-warning-option
    -fPIC
    -march=native
)

set(FUNC_TEST_PATH "test/func_test/func_test.cpp")
string(REPLACE ${FUNC_TEST_PATH} "" nogdb_API_TEST "${nogdb_API_TEST}")
add_library(nogdb_func_test_object OBJECT EXCLUDE_FROM_ALL
    ${nogdb_API_TEST}
    ${nogdb_API_TEST_HEADER}
)
target_include_directories(nogdb_func_test_object PRIVATE $<TARGET_PROPERTY:nogdb,INTERFACE_INCLUDE_DIRECTORIES>)

# func_test_executable(name)
function(func_test_executable name)
    string(TOUPPER ${name} upper_name)
    add_executable(func_test_${name} EXCLUDE_FROM_ALL ${FUNC_TEST_PATH} $<TARGET_OBJECTS:nogdb_func_test_object>)
    target_link_libraries(func_test_${name} nogdb)
    target_compile_options(func_test_${name}
        PRIVATE
            ${FUNC_TEST_COMPILE_OPTIONS}
            -DTEST_${upper_name}_OPERATIONS
    )
    add_test(build_func_test_${name} "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --target func_test_${name})
    add_test(NAME ${name}_test COMMAND func_test_${name})
    set_tests_properties(${name}_test PROPERTIES DEPENDS ctest_build_test_${name})
endfunction()

if(nogdb_BuildTests)
    enable_testing()
    func_test_executable(context)
    func_test_executable(schema)
    func_test_executable(record)
    func_test_executable(misc)
    func_test_executable(graph)
    func_test_executable(find)
    func_test_executable(inheritance)
    func_test_executable(index)
    func_test_executable(schema_txn)
    func_test_executable(txn)
    func_test_executable(sql)

    add_executable(unit_test_all EXCLUDE_FROM_ALL
        ${nogdb_UNIT_TEST}
        ${nogdb_UNIT_TEST_HEADER}
        ${nogdb_UNIT_TEST_LMDB}
        ${nogdb_UNIT_TEST_LMDB_HEADER}
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src/datatype.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/utils.cpp
    )
    target_include_directories(unit_test_all
        PRIVATE
            $<TARGET_PROPERTY:nogdb,INTERFACE_INCLUDE_DIRECTORIES>
    )
    find_library(gtest_LIB gtest)
    find_library(gmock_LIB gmock)
    target_link_libraries(unit_test_all lmdb ${gtest_LIB} ${gmock_LIB} pthread)
    if (NOT APPLE)
        target_link_libraries(unit_test_all atomic)
    endif()
    add_test(build_unit_test_all "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --target unit_test_all)
    add_test(NAME unit_test_all COMMAND unit_test_all)
endif()


## TARGET install
install(TARGETS nogdb DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)
install(DIRECTORY lib/lmdb/ DESTINATION include/nogdb/lmdb
    FILES_MATCHING PATTERN "lib/lmdb/*.h"
)