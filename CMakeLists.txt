cmake_minimum_required(VERSION 3.12)
project(Lab2 C)

set(CMAKE_C_STANDARD 99)

add_executable(Lab2 main.c testing_sketches/imageCaptureSketch/optfft.h testing_sketches/imageCaptureSketch/optfft.c testing_sketches/imageCaptureSketch/tables.c)