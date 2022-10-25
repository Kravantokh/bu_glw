
cmake_minimum_required(VERSION 3.24.1)

project(bu_glw VERSION 1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

find_package(OpenGL)

set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

message(STATUS "${OPENGL_INCLUDE_DIR}")

add_library(bu_glw src/bu_glw.cpp)

target_include_directories(bu_glw INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(bu_glw PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${OPENGL_INCLUDE_DIR})
