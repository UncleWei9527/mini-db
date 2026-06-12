cmake_minimum_required(VERSION 4.2)
project(db)

set(CMAKE_CXX_STANDARD 20)
add_executable(db main.cpp
        type_id.h
        value.cpp
        value.h)
