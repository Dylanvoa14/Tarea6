cmake_minimum_required(VERSION 3.16)
project(Tarea6_Arboles CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

add_executable(tarea6 src/main.cpp)
target_include_directories(tarea6 PRIVATE include)
