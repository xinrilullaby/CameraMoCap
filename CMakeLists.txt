cmake_minimum_required(VERSION 3.10.0)

project(CameraMoCap VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)

add_subdirectory(./src)

target_include_directories(main PUBLIC ./include)
set(includePath ./include)

find_library(OPENGL32_LIB_PATH  Opengl32)
find_library(GDI32_LIB_PATH     gdi32.lib)
