cmake_minimum_required(VERSION 3.16)
project(filegate_implementation LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 严格编译告警卡点
add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
    -Werror=return-type
    -O2
    -pthread
)

include_directories(include)

add_executable(filegate_demo
    src/main.cpp
    src/storage_engine.cpp
    src/watermark_engine.cpp
)

target_link_libraries(filegate_demo PRIVATE pthread)
