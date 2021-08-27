cmake_minimum_required(VERSION 3.20)
project(twitter)

set(CMAKE_CXX_STANDARD 23)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(twitter main.cpp Connection.cpp Connection.h)
target_link_libraries(twitter PRIVATE Threads::Threads)