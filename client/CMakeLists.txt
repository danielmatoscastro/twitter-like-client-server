cmake_minimum_required(VERSION 3.16)
project(twitter)

set(CMAKE_CXX_STANDARD 20)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(twitter main.cpp Connection.cpp Connection.h)
target_link_libraries(twitter PRIVATE Threads::Threads)
