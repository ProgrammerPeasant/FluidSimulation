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
        Fixed.h
        specialArr.h
        originalFunctions.h
        typeHolder.h
        random.h
        arraysTemps.h
        types.h
        generatorFactory.h
)

add_executable(FluidFinalVer ${SOURCES} ${HEADERS})

#set(TYPES_ENCODED "RkxPQVQsRE9VQkxFLEZJWEVEKDMyLDkpLEZBU1RfRklYRUQoNDAsNSksRkFTVF9GSVhFRCg1MCwxNikK")
#set(SIZES_ENCODED "UygxMCwyNTApLFMoMzYsODQp")
#
#function(decode_base64 INPUT_STRING OUTPUT_VARIABLE)
#    execute_process(
#            COMMAND python -c "import base64; print(base64.b64decode('${INPUT_STRING}').decode('utf-8'))"
#            OUTPUT_VARIABLE decoded_string
#            OUTPUT_STRIP_TRAILING_WHITESPACE
#    )
#    set(${OUTPUT_VARIABLE} "${decoded_string}" PARENT_SCOPE)
#endfunction()
#
#decode_base64("${TYPES_ENCODED}" DECODED_TYPES)
#decode_base64("${SIZES_ENCODED}" DECODED_SIZES)
#
#message(STATUS "Decoded DTYPES: ${DECODED_TYPES}")
#message(STATUS "Decoded DSIZES: ${DECODED_SIZES}")
#
#target_compile_definitions(FluidFinalVer PRIVATE
#        DTYPES=${DECODED_TYPES}
#        DSIZES=${DECODED_SIZES}
#)

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
