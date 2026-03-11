cmake_minimum_required(VERSION 4.2)
project(mlc)

set(CMAKE_CXX_STANDARD 23)

file(GLOB_RECURSE SRC "Modules/*.cpp")
file(GLOB_RECURSE SRC_M "Modules/*.cppm")

add_executable(mlc main.cpp)

add_library(mlc_m ${SRC})
target_sources(mlc_m PUBLIC FILE_SET CXX_MODULES FILES ${SRC_M})

target_link_libraries(mlc PRIVATE mlc_m)

include(Test/test.cmake)

include(Public/public.cmake)

target_link_libraries(mlc_m PUBLIC public)


add_compile_options(-Wshadow)


