
cmake_minimum_required(VERSION 3.24.1)

project(bu_glw VERSION 1.0)

set(CMAKE_CXX_STANDARD 11)


add_library(bu_glw STATIC ${CMAKE_CURRENT_SOURCE_DIR}/src/bu_glw.cpp)

target_include_directories(bu_glw INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(bu_glw PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
