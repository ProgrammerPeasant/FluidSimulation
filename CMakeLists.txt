cmake_minimum_required(VERSION 3.27)

project(FluidFinalVer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wa,-mbig-obj -w -Os -g0")

set(DTYPES "FLOAT,DOUBLE,FIXED(32,9),FAST_FIXED(40,5),FAST_FIXED(50,16)")
set(DSIZES "S(10,250),S(36,84)")

set(SOURCES
        main.cpp
)

set(HEADERS
        Fixed.hpp
        specialArr.hpp
        originalFunctions.hpp
        typeHolder.hpp
        random.hpp
        arraysTemps.hpp
        types.hpp
        generatorFactory.hpp
)

add_executable(FluidFinalVer ${SOURCES} ${HEADERS})

string(REPLACE "(" "\(" ESCAPED_TYPES "${DTYPES}")
string(REPLACE ")" "\)" ESCAPED_TYPES "${ESCAPED_TYPES}")

string(REPLACE "(" "\(" ESCAPED_SIZES "${DSIZES}")
string(REPLACE ")" "\)" ESCAPED_SIZES "${ESCAPED_SIZES}")

target_compile_definitions(FluidFinalVer PRIVATE
        DTYPES=${ESCAPED_TYPES}
        DSIZES=${ESCAPED_SIZES}
)

target_compile_options(FluidFinalVer PRIVATE
        -Wno-unused-variable
)


target_include_directories(FluidFinalVer PRIVATE ${CMAKE_SOURCE_DIR})

message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "CMake Version: ${CMAKE_VERSION}")
message(STATUS "Target: FluidFinalVer")
