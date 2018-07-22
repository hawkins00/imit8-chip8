cmake_minimum_required(VERSION 3.10)
project(imit8_chip8)

set(CMAKE_CXX_STANDARD 11)

add_executable(imit8_chip8 src/main.cpp src/chip8.cpp src/chip8.h src/logWriter.cpp src/logWriter.h src/display.cpp src/display.h)