cmake_minimum_required(VERSION 3.20)
project(twitter)

set(CMAKE_CXX_STANDARD 23)

add_subdirectory("${PROJECT_SOURCE_DIR}/client" "${PROJECT_SOURCE_DIR}/client/client_output")
add_subdirectory("${PROJECT_SOURCE_DIR}/server" "${PROJECT_SOURCE_DIR}/server/server_output")
