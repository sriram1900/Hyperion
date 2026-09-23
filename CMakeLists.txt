cmake_minimum_required(VERSION 3.20)

project(Hyperion LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(hyperion
    hyperion.cpp
)

target_compile_options(hyperion PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

find_package(Threads REQUIRED)
target_link_libraries(hyperion PRIVATE Threads::Threads)