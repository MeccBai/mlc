cmake_minimum_required(VERSION 4.2)
project(mlc)

set(CMAKE_CXX_STANDARD 23)

file(GLOB_RECURSE SRC "Modules/*.cpp")
file(GLOB_RECURSE SRC_M "Modules/*.cppm")

add_executable(mlc main.cpp ${SRC})
target_sources(mlc PUBLIC FILE_SET CXX_MODULES FILES ${SRC_M})
