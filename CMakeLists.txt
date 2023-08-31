cmake_minimum_required(VERSION 3.25)
project(tiny-renderer)

set(CMAKE_CXX_STANDARD 17)

add_executable(tiny-renderer main.cpp tgaimage.cpp tgaimage.h geometry.h model.cpp model.h)
