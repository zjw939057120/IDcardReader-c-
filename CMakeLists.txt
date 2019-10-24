cmake_minimum_required(VERSION 3.13)
project(idcard)

#head file path
INCLUDE_DIRECTORIES(
        include
)

set(CMAKE_CXX_STANDARD 14)

#add executable file
add_executable(
        idcard
        main.cpp
        base64.cpp)