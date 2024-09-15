cmake_minimum_required(VERSION 3.10)
project(bf
        DESCRIPTION "brainfuck interpreter"
        LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_executable(bf main.cpp)
