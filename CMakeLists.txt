cmake_minimum_required(VERSION 3.12)

project(
        fixed-floating
        VERSION 1.0
        DESCRIPTION "C++ Year 2024 computer_hardware_1"
        LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)

add_executable(${PROJECT_NAME} ../main.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR})