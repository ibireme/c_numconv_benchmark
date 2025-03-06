cmake_minimum_required(VERSION 3.14)

project(c_numconv_benchmark)

# ------------------------------------------------------------------------------
# build type
if(XCODE OR MSVC)
    set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)
endif()
if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, default to: Release")
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus")
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
endif()



# ------------------------------------------------------------------------------
# native arch
option(ARCH_NATIVE "Enable native arch" OFF)
if(ARCH_NATIVE AND CMAKE_C_COMPILER_ID MATCHES "GNU|Clang|Intel|MSVC")
    include(CheckCCompilerFlag)
    include(CheckCSourceRuns)

    set(ISA_LIST
        "AVX512|/arch:AVX512|-mavx512f|#include <immintrin.h>\nint main(){__m512i a = _mm512_setzero_si512()\\\;return 0\\\;}"
        "AVX2|/arch:AVX2|-mavx2|#include <immintrin.h>\nint main(){__m256i a = _mm256_setzero_si256()\\\;return 0\\\;}"
        "SSE2|/arch:SSE2|-msse2|#include <immintrin.h>\nint main(){__m128i a = _mm_setzero_si128()\\\;return 0\\\;}"
    )

    foreach(ISA_INFO IN LISTS ISA_LIST)
        string(REPLACE "|" ";" ISA_INFO "${ISA_INFO}")
        list(GET ISA_INFO 0 ISA_NAME)
        if(MSVC)
            list(GET ISA_INFO 1 ISA_FLAG)
        else()
            list(GET ISA_INFO 2 ISA_FLAG)
        endif()
        list(GET ISA_INFO 3 ISA_CHECK_CODE)

        message(STATUS "Check ${ISA_NAME} support")
        file(WRITE "${CMAKE_BINARY_DIR}/${ISA_NAME}_check.c" "${ISA_CHECK_CODE}")
        try_run(RUN_RESULT COMPILE_RESULT
            "${CMAKE_BINARY_DIR}/${ISA_NAME}_check"
            "${CMAKE_BINARY_DIR}/${ISA_NAME}_check.c"
            COMPILE_DEFINITIONS "${ISA_FLAG}"
            COMPILE_OUTPUT_VARIABLE COMPILE_OUTPUT
            RUN_OUTPUT_VARIABLE RUN_OUTPUT
        )

        if(COMPILE_RESULT AND RUN_RESULT EQUAL 0)
            if(NOT ISA_FLAG_NATIVE)
                set(ISA_FLAG_NATIVE ${ISA_FLAG})
            endif()
            add_definitions(-DHAVE_${ISA_NAME})
            message(STATUS "Define HAVE_${ISA_NAME}=1")
        endif()
    endforeach()

    if(MSVC AND ISA_FLAG_NATIVE)
        add_compile_options(${ISA_FLAG_NATIVE})
    elseif(NOT MSVC)
        add_compile_options(-march=native)
    endif()
endif()



# ------------------------------------------------------------------------------
# vendor
set(vendors "")

# fmt
add_subdirectory(vendor/fmt)
list(APPEND vendors "fmt")

# google double conversion
add_subdirectory(vendor/double-conversion)
list(APPEND vendors "double-conversion")

# lemire fast double parser (header only)
add_library(fast_double_parser INTERFACE)
target_include_directories(fast_double_parser INTERFACE "vendor/fast_double_parser/include")
list(APPEND vendors "fast_double_parser")

# dragonbox
add_subdirectory(vendor/dragonbox)
list(APPEND vendors "dragonbox_to_chars")

# grisu-exact
add_library(grisu_exact STATIC
    "vendor/Grisu-Exact/grisu_exact.h"    
    "vendor/Grisu-Exact/fp_to_chars.h"
    "vendor/Grisu-Exact/fp_to_chars/fp_to_chars.cpp"
)
target_include_directories(grisu_exact INTERFACE "vendor/Grisu-Exact")
list(APPEND vendors "grisu_exact")

# schubfach
file(GLOB SOURCES "vendor/Drachennest/src/*.cc" "vendor/Drachennest/src/*.h")
add_library(schubfach STATIC ${SOURCES})
target_include_directories(schubfach INTERFACE "vendor/Drachennest/src")
list(APPEND vendors "schubfach")

# erthink (header only)
add_library(erthink INTERFACE)
target_include_directories(erthink INTERFACE "vendor/erthink")
list(APPEND vendors "erthink")

# ryu
if(NOT MSVC)
    file(GLOB SOURCES 
        "vendor/ryu/ryu/*.h"
        "vendor/ryu/ryu/*.c"
    )
    add_library(ryu STATIC ${SOURCES})
    target_include_directories(ryu PUBLIC "vendor/ryu")
    list(APPEND vendors "ryu")
endif()

# david gay
add_library(david_gay STATIC 
    "vendor/david_gay/david_gay_dtoa.h"
    "vendor/david_gay/david_gay_dtoa.c"
)
target_include_directories(david_gay PUBLIC "vendor/david_gay")
list(APPEND vendors "david_gay")

# swift dtoa
add_library(swift_dtoa STATIC 
    "vendor/swift_dtoa/SwiftDtoa.h"
    "vendor/swift_dtoa/SwiftDtoa.cpp"
)
target_include_directories(swift_dtoa PUBLIC "vendor/swift_dtoa")
list(APPEND vendors "swift_dtoa")

# yy_double
add_library(yy_double STATIC 
    "vendor/yy_double/yy_double.h"
    "vendor/yy_double/yy_double.c"
)
target_include_directories(swift_dtoa PUBLIC "vendor/yy_double")
list(APPEND vendors "yy_double")

# yybench
add_subdirectory(vendor/yybench)
list(APPEND vendors "yybench")


# ------------------------------------------------------------------------------
# runs
set(run_names "itoa" "dtoa" "atoi" "strtod")
foreach(run_name ${run_names})

    # create exe
    add_executable("run_${run_name}" "")

    # add source files
    file(GLOB SOURCES 
        "src/${run_name}/*.h"
        "src/${run_name}/*.c" 
        "src/${run_name}/*.cpp"
    )
    list(APPEND SOURCES "src/main/main.c")
    target_sources("run_${run_name}" PRIVATE ${SOURCES})
    target_include_directories("run_${run_name}" PRIVATE "src/${run_name}")

    # add links
    target_link_libraries("run_${run_name}" PRIVATE ${vendors})

    # align all functions 64 bytes
    if (CMAKE_C_COMPILER_ID MATCHES "Clang")
        target_compile_options("run_${run_name}" PRIVATE "-mllvm" "-align-all-functions=7")
    elseif (CMAKE_C_COMPILER_ID MATCHES "GNU")
        target_compile_options("run_${run_name}" PRIVATE "-falign-functions=64")
    endif()
    
endforeach()

# config project
if(MSVC)
    if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
        string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    endif()
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-DBENCHMARK_DATA_PATH="${CMAKE_CURRENT_BINARY_DIR}")

elseif(XCODE)
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
    include(XcodeProperty)

    
    foreach(run_name ${run_names})
        set_default_xcode_property("run_${run_name}")
        set_xcode_deployment_version("run_${run_name}" "10.11" "9.0" "9.0" "2.0")
        set_xcode_language_standard("run_${run_name}" "gnu11" "gnu++17")
        if(NOT IOS)
            set_xcode_property("run_${run_name}" OTHER_CFLAGS[variant=Debug] -DBENCHMARK_DATA_PATH="\\\\\"${CMAKE_CURRENT_BINARY_DIR}\\\\\"")
            set_xcode_property("run_${run_name}" OTHER_CFLAGS[variant=Release] -DBENCHMARK_DATA_PATH="\\\\\"${CMAKE_CURRENT_BINARY_DIR}\\\\\"")
        endif()
    endforeach()

    foreach(vendor ${vendors})
        get_property(vendor_type TARGET ${vendor} PROPERTY TYPE)
        if(NOT vendor_type STREQUAL "INTERFACE_LIBRARY")
            set_default_xcode_property(${vendor})
            set_xcode_deployment_version(${vendor} "10.11" "9.0" "9.0" "2.0")
            set_xcode_language_standard(${vendor} "gnu11" "gnu++17")
        endif()
    endforeach()

else()
    add_definitions(-DBENCHMARK_DATA_PATH="${CMAKE_CURRENT_BINARY_DIR}")
endif()

# copy test data
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
