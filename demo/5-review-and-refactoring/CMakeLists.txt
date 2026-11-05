cmake_minimum_required(VERSION 3.16)
project(filegate_phase5_refactoring CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/legacy
    ${CMAKE_CURRENT_SOURCE_DIR}/refactored
    ${CMAKE_CURRENT_SOURCE_DIR}/../3-high-quality-implementation/include
)

add_executable(test_refactoring_equivalence
    tests/test_refactoring_equivalence.cpp
)
