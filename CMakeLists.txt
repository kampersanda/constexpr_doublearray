cmake_minimum_required(VERSION 3.2)
project(constexpr_doublearray VERSION 1.0.0 LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE)
    # set(CMAKE_BUILD_TYPE "Debug")
    set(CMAKE_BUILD_TYPE "Release")
endif(NOT CMAKE_BUILD_TYPE)

if ((CMAKE_CXX_COMPILER_ID STREQUAL "Clang") OR (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang"))
    set(CMAKE_COMPILER_IS_CLANGXX 1)
endif ()
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_COMPILER_IS_GNUCXX 1)
endif ()

# C++17 compiler check
if ((CMAKE_COMPILER_IS_GNUCXX AND ${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS 7.0) OR (CMAKE_COMPILER_IS_CLANGXX AND ${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS 4.0))
    message(FATAL_ERROR "Your C++ compiler does not support C++17. Please install g++ 7.0 (or greater) or clang 4.0 (or greater)")
else ()
    message(STATUS "Compiler is recent enough to support C++17.")
endif ()

if (CMAKE_COMPILER_IS_CLANGXX)
    # https://qiita.com/ushitora_anqou/items/6322c6839f39f6b03a4e
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fconstexpr-depth=-1 -fconstexpr-steps=-1")
endif ()
if (CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fconstexpr-depth=2147483647 -fconstexpr-loop-limit=2147483647")
endif ()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z -pthread -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG -march=native -O3")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address -fno-omit-frame-pointer -O0 -g -DDEBUG")

message(STATUS "BUILD_TYPE is ${CMAKE_BUILD_TYPE}")
message(STATUS "CXX_FLAGS are ${CMAKE_CXX_FLAGS}")
message(STATUS "CXX_FLAGS_DEBUG are ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CXX_FLAGS_RELEASE are ${CMAKE_CXX_FLAGS_RELEASE}")

add_executable(sample sample.cpp)
add_executable(demo demo.cpp)
