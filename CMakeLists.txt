cmake_minimum_required(VERSION 3.14)
project(PPM_converter C)

set(CMAKE_C_STANDARD 99)

SET(CMAKE_EXE_LINKER_FLAGS "-lm -pthread")

add_executable(ppm_converter main.c pam.c pam.h pam_converters.c pam_converters.h sobel.c sobel.h)