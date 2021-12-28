cmake_minimum_required(VERSION 3.7)
project(Client C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=thread -g -OX")

add_executable(Client main.c Mravec.h)

target_link_libraries(Client pthread)

