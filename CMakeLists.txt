cmake_minimum_required(VERSION 3.23)
project(vimeo_link)

set(CMAKE_CXX_STANDARD 17)

add_executable(vimeo_link main.cpp)

target_link_libraries(vimeo_link curl)