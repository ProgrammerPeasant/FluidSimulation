cmake_minimum_required(VERSION 3.27)
project(FluidFinalVer)

set(CMAKE_CXX_STANDARD 20)

add_executable(FluidFinalVer main.cpp
        Fluid.h
        specialArr.h
        engine.h)
