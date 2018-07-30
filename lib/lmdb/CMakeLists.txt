cmake_minimum_required(VERSION 3.5.1)

project(lmdb LANGUAGES C)

set(lmdb_SOURCE mdb.c midl.c)

add_library(lmdb_OBJ OBJECT ${lmdb_SOURCE})
add_library(lmdb $<TARGET_OBJECTS:lmdb_OBJ>)
target_include_directories(lmdb
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)
target_compile_options(lmdb
    PRIVATE
        -W
        -Wall
        -Wno-unused-parameter
        -Wno-format-extra-args
        -pthread
)