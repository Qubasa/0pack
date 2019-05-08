cmake_minimum_required(VERSION 3.02)

## Find all source files to be compiled into object files
file( GLOB SOURCES *.cpp )
add_executable("${BINNAME}" main.cpp)
