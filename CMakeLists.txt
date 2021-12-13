cmake_minimum_required(VERSION 3.8)
project(BeXlib)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-register")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib")

include_directories(include)
add_subdirectory(xlib)
add_subdirectory(xext)
add_subdirectory(test)
