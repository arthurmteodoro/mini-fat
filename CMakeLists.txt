cmake_minimum_required(VERSION 3.15)
project(mini_fat)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_STANDARD_LIBRARIES  "-lm")

add_compile_definitions(USING_ARDUINO=0)

add_executable(mini_fat main.c minifat.c minifat.h serial/serial.c serial/serial.h)
