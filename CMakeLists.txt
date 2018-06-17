cmake_minimum_required(VERSION 3.5.1)

project(nogdb VERSION 1.0.0 LANGUAGES C CXX)

file(GLOB nogdb_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB nogdb_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp)
file(GLOB nogdb_PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
file(GLOB nogdb_TEST ${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp)
file(GLOB nogdb_TEST_MODULE ${CMAKE_CURRENT_SOURCE_DIR}/test/module/*.cpp)
file(GLOB nogdb_TEST_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/test/*.h)
file(GLOB nogdb_TEST_MODULE_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/test/module/*.h)

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

## TARGET test
include(${CMAKE_CURRENT_SOURCE_DIR}/test/CMakeLists.txt)
include(${CMAKE_CURRENT_SOURCE_DIR}/test/module/CMakeLists.txt)

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
