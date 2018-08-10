cmake_minimum_required(VERSION 3.10)
project(imit8_chip8)

set(CMAKE_CXX_STANDARD 11)

add_executable(imit8_chip8 src/main.cpp src/Chip8.cpp src/Chip8.h src/LogWriter.cpp src/LogWriter.h src/Display.cpp src/Display.h)