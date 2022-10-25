cmake_minimum_required(VERSION 3.24.1)

project(bu_glw VERSION 1.0)

cmake_policy(SET CMP0072 NEW)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

find_package( OpenGL REQUIRED )
find_package(Python COMPONENTS Interpreter)

#Handling Python and gl3w download
if(NOT Python_Interpreter_FOUND)
	message(FATAL_ERROR "Python could not be found! The compilation can not proceed without a Python interpreter! (gl3w will be missing)")
endif()

if (WIN32)
	#Apparently on windows despite having python3 installed it would not recognize python3 as a command, so only python should be run.
	message("Successfully found Python. Running download script for gl3w.")
	execute_process(COMMAND python gl3w_gen.py --root ${PROJECT_BINARY_DIR}/lib/gl3w WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/lib/gl3w)

#On linux the command does matter, thus it is tested which version to use.
elseif(${Python_VERSION} GREATER_EQUAL 3.0)
	message("Successfully found Python3. Running download script for gl3w.")
	execute_process(COMMAND python3 gl3w_gen.py --root ${PROJECT_BINARY_DIR}/lib/gl3w WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/lib/gl3w)
elseif(${Python_VERSION} LESS 3.0)
	message("Successfully found Python2. Running download script for gl3w.")
	execute_process(COMMAND python gl3w_gen.py --root ${PROJECT_BINARY_DIR}/lib/gl3w WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/lib/gl3w)
endif()

include_directories(${PROJECT_BINARY_DIR}/lib/gl3w/include)

add_library(bu_glw src/bu_glw.cpp)

add_subdirectory(${PROJECT_SOURCE_DIR}/lib/gl3w)

target_link_libraries(bu_glw gl3w)

target_include_directories(bu_glw INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
	#                                         ${PROJECT_BINARY_DIR}/lib/glw3/include
	#								 ${CMAKE_SOURCE_DIR}/lib/glw3/include
	#                                   ${CMAKE_CURRENT_SOURCE_DIR}/include
                          )
target_include_directories(bu_glw PUBLIC ${OPENGL_INCLUDE_DIR}
                                         ${PROJECT_BINARY_DIR}/lib/glw3/include
                                         ${CMAKE_CURRENT_SOURCE_DIR}/include
                          )
