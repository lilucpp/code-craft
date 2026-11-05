cmake_minimum_required(VERSION 3.16)
project(AntQtDemos LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets Test)

# Demo 1: Component Gallery
add_executable(demo1_component_gallery
    demo1_component_gallery/main.cpp
    resources.qrc
)
target_link_libraries(demo1_component_gallery PRIVATE Qt5::Widgets Qt5::Test)

# Demo 2: Pro Query Table with Drawer
add_executable(demo2_pro_query_table
    demo2_pro_query_table/main.cpp
    resources.qrc
)
target_link_libraries(demo2_pro_query_table PRIVATE Qt5::Widgets Qt5::Test)
