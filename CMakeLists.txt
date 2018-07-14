cmake_minimum_required(VERSION 3.10)
project(imit8_chip8)

set(CMAKE_CXX_STANDARD 11)

add_executable(imit8_chip8 main.cpp chip8.cpp chip8.h logWriter.cpp logWriter.h)