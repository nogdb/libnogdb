cmake_minimum_required(VERSION 3.8.2)

project(nogdb VERSION 0.10.0 LANGUAGES CXX)

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
    -fPIC
)

enable_testing()

## TARGET lmdb
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/lmdb)

## TARGET nogdb
add_library(nogdb
    ${nogdb_SOURCE}
    ${nogdb_HEADER}
    ${nogdb_PUBLIC}
    $<TARGET_OBJECTS:lmdb_OBJ>
)
add_dependencies(nogdb
    lmdb_OBJ
)
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

## TARGET test
#add_executable(runtest EXCLUDE_FROM_ALL
#    ${nogdb_TEST} ${nogdb_TEST_HEADER}
#)
#target_link_libraries(runtest
#    nogdb
#)
#target_compile_options(runtest
#    PRIVATE
#        ${COMPILE_OPTIONS}
#        -Wno-unused-but-set-variable
#        -Wno-unknown-warning-option
#)
#add_test(build_runtest "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --target runtest)
#add_test(NAME alltest COMMAND runtest)
#set_tests_properties(alltest PROPERTIES DEPENDS ctest_build_my_test)
#add_executable(runtest_f EXCLUDE_FROM_ALL
#    ${nogdb_TEST} ${nogdb_TEST_HEADER}
#)

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