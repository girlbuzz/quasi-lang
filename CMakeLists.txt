cmake_minimum_required(VERSION 3.12)

project(quasi-lang)

set(CMAKE_CXX_STANDARD 17)

add_executable(quasi src/main.cpp src/Expression.cpp src/Lexicon.cpp src/Function.cpp src/Source.cpp)
