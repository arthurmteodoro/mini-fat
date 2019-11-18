cmake_minimum_required(VERSION 3.15)
project(mini_fat)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_STANDARD_LIBRARIES  "-lm")

add_executable(mini_fat main.c minifat.c minifat.h)
