cmake_minimum_required(VERSION 3.20)
project(twitter)

set(CMAKE_CXX_STANDARD 23)

add_executable(twitter main.cpp Connection.cpp Connection.h)
