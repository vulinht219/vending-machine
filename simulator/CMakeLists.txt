cmake_minimum_required(VERSION 3.20)

project(candy_simulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)

pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)

add_executable(candy-simulator
    src/main.cpp
)

target_link_libraries(candy-simulator PRIVATE
    PkgConfig::SDL2
)