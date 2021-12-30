cmake_minimum_required(VERSION 3.0)
project(Client C)

set(CMAKE_C_STANDARD 99)

add_executable(Client main.c Mravec.h)

target_link_libraries(Client pthread)



