cmake_minimum_required(VERSION 4.2)
project(Strategeti)

set(CMAKE_CXX_STANDARD 20)
# set(CMAKE_CXX_FLAGS -mbmi)

add_executable(Strategeti main.cpp
        Zobrist.cpp
        Evaluate.h
)
