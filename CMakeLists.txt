cmake_minimum_required(VERSION 3.8)
project(BeXlib)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-register")

include_directories(include)
add_subdirectory(xlib)
add_subdirectory(test)
