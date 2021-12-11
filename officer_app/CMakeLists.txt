cmake_minimum_required(VERSION 3.10)
project(zad1 C)

set(CMAKE_C_STANDARD 11)

add_executable(off_app
        officer.c)
target_link_libraries(off_app pam pam_misc)