cmake_minimum_required(VERSION 3.21)
project(zad1 C)

set(CMAKE_C_STANDARD 11)

add_executable(zad1
        officer.c)
target_link_libraries(zad1 pam pam_misc)