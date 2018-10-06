cmake_minimum_required(VERSION 3.5.1)

project(nogdb VERSION 0.12.0 LANGUAGES C CXX)

file(GLOB nogdb_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB nogdb_SOURCE_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp)
file(GLOB nogdb_PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/nogdb/*.h)
file(GLOB nogdb_API_TEST ${CMAKE_CURRENT_SOURCE_DIR}/test/apitest/*.cpp)
file(GLOB nogdb_API_TEST_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/test/apitest/*.h)
file(GLOB nogdb_UNIT_TEST ${CMAKE_CURRENT_SOURCE_DIR}/test/unittest/*.cpp)
file(GLOB nogdb_UNIT_TEST_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/test/unittest/*.h)
file(GLOB nogdb_UNIT_TEST_LMDB ${CMAKE_CURRENT_SOURCE_DIR}/test/unittest/lmdb_engine/*.cpp)
file(GLOB nogdb_UNIT_TEST_LMDB_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/test/unittest/lmdb_engine/*.h)

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

## TARGET unittest
set(UNITTEST_COMPILE_OPTIONS
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

## TARGET apitest
set(APITEST_COMPILE_OPTIONS
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

set(APITEST_PATH "test/apitest/apitest.cpp")
string(REPLACE ${APITEST_PATH} "" nogdb_API_TEST "${nogdb_API_TEST}")
add_library(nogdb_apitest_object OBJECT EXCLUDE_FROM_ALL
    ${nogdb_API_TEST}
    ${nogdb_API_TEST_HEADER}
)
target_include_directories(nogdb_apitest_object PRIVATE $<TARGET_PROPERTY:nogdb,INTERFACE_INCLUDE_DIRECTORIES>)

# apitest_executable(name)
function(apitest_executable name)
    string(TOUPPER ${name} upper_name)
    add_executable(apitest_${name} EXCLUDE_FROM_ALL ${APITEST_PATH} $<TARGET_OBJECTS:nogdb_apitest_object>)
    target_link_libraries(apitest_${name} nogdb)
    target_compile_options(apitest_${name}
        PRIVATE
            ${APITEST_COMPILE_OPTIONS}
            -DTEST_${upper_name}_OPERATIONS
    )
    add_test(build_apitest_${name} "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --target apitest_${name})
    add_test(NAME test_${name} COMMAND apitest_${name})
    set_tests_properties(test_${name} PROPERTIES DEPENDS ctest_build_test_${name})
endfunction()

if(nogdb_BuildTests)
    enable_testing()
    apitest_executable(context)
    apitest_executable(schema)
    apitest_executable(record)
    apitest_executable(misc)
    apitest_executable(graph)
    apitest_executable(find)
    apitest_executable(inheritance)
    apitest_executable(index)
    apitest_executable(schema_txn)
    apitest_executable(txn)
    apitest_executable(sql)

    add_executable(unittest_all EXCLUDE_FROM_ALL
        ${nogdb_UNIT_TEST}
        ${nogdb_UNIT_TEST_HEADER}
        ${nogdb_UNIT_TEST_LMDB}
        ${nogdb_UNIT_TEST_LMDB_HEADER}
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src/datatype.cpp
    )
    target_include_directories(unittest_all
        PRIVATE
            $<TARGET_PROPERTY:nogdb,INTERFACE_INCLUDE_DIRECTORIES>
    )
    find_library(gtest_LIB gtest)
    find_library(gmock_LIB gmock)
    target_link_libraries(unittest_all lmdb ${gtest_LIB} ${gmock_LIB} pthread)
    if (NOT APPLE)
        target_link_libraries(unittest_all atomic)
    endif()
    add_test(build_unittest_all "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --target unittest_all)
    add_test(NAME unittest_all COMMAND unittest_all)
endif()


## TARGET install
install(TARGETS nogdb DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)
install(DIRECTORY lib/lmdb/ DESTINATION include/nogdb/lmdb
    FILES_MATCHING PATTERN "lib/lmdb/*.h"
)