cmake_minimum_required(VERSION 3.23)
project(dropout-dl)

set(CMAKE_CXX_STANDARD 17)

add_executable(dropout-dl main.cpp)

target_link_libraries(dropout-dl curl sqlite3)