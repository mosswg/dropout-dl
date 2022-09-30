cmake_minimum_required(VERSION 3.23)
project(dropout-dl-tests)

set(CMAKE_CXX_STANDARD 17)

add_executable(test test.cpp episode_tests.cpp ../src/episode.cpp)

target_link_libraries(test curl sqlite3 gcrypt)

message(STATUS "Building Tests")

include_directories(episode ../src)
