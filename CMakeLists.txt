cmake_minimum_required(VERSION 3.2.2)

# set clang as our default compiler
if (("$ENV{CC}" STREQUAL "") OR ("$ENV{CXX}" STREQUAL ""))
    set( CMAKE_C_COMPILER clang-3.6 )
    set( CMAKE_CXX_COMPILER clang++-3.6 )
endif()

project(MajorProject)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
link_directories(/usr/local/lib)
include_directories(/usr/local/include)

execute_process(COMMAND pkg-config --libs opencv 
				OUTPUT_VARIABLE OPENCV_LIBS 
				OUTPUT_STRIP_TRAILING_WHITESPACE)

add_executable(StreamingDemo StreamingDemo.cc)
target_link_libraries(StreamingDemo ${OPENCV_LIBS})