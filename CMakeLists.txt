cmake_minimum_required(VERSION 3.25)
project(tiny-renderer)

set(CMAKE_CXX_STANDARD 20)

add_executable(tiny-renderer
        main.cpp
        tgaimage.h tgaimage.cpp
        model.h model.cpp
        geometry.h geometry.cpp
        our_gl.h our_gl.cpp
        shaders.txt)
