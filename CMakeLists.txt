cmake_minimum_required(VERSION 3.2.2)

# set clang as our default compiler
if (("$ENV{CC}" STREQUAL "") OR ("$ENV{CXX}" STREQUAL ""))
    set( CMAKE_C_COMPILER clang-3.8 )
    set( CMAKE_CXX_COMPILER clang++-3.8 )
endif()

project(MajorProject)

find_program( CLANG_FORMAT_PATH NAMES clang-format-3.6 clang-format )
if( ${CLANG_FORMAT_PATH} MATCHES "CLANG_FORMAT_PATH-NOTFOUND")
MESSAGE( WARNING "clang-format 3.6 not installed.")
MESSAGE( FATAL_ERROR "Try running 'sudo apt-get install clang-format-3.6'")
endif()

# Gets and prints clang format version
execute_process( COMMAND ${CLANG_FORMAT_PATH} -version
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                OUTPUT_STRIP_TRAILING_WHITESPACE
                OUTPUT_VARIABLE CLANG_FORMAT_VERSION )
MESSAGE( STATUS "Formatting Code with ${CLANG_FORMAT_VERSION}")

file(GLOB_RECURSE FORMAT_FILES *.cc *.h *.cu)

execute_process( COMMAND ${CLANG_FORMAT_PATH} -style=file -i ${FORMAT_FILES} )

set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_C_FLAGS_DEBUG "-g -ggdb -O0")
set(CMAKE_CXX_FLAGS_DEBUG "-g -ggdb -O0")
set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} " -Wall -Werror -Wstrict-aliasing=2 -fPIC -std=c14 " )
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} " -Wall -Werror -Wstrict-aliasing=2 -fPIC -std=c++14 " )

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
link_directories(/usr/local/lib)
include_directories(/usr/local/include/
					/usr/local/include/tensorflow
					/usr/local/include/tensorflow/bazel-genfiles
					/usr/local/include/tensorflow/third_party)

execute_process(COMMAND pkg-config --libs opencv 
				OUTPUT_VARIABLE OPENCV_LIBS 
				OUTPUT_STRIP_TRAILING_WHITESPACE)

add_executable(StreamingDemo StreamingDemo.cc)
target_link_libraries(StreamingDemo ${OPENCV_LIBS})
target_link_libraries(StreamingDemo tensorflow_cc)