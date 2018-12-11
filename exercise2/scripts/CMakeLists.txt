cmake_minimum_required(VERSION 3.10)
project(scripts C)

set(CMAKE_C_STANDARD 99)

add_executable(scripts forksum.c)
target_link_libraries(forksum m)