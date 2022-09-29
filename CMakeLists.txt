cmake_minimum_required(VERSION 3.23)
project(dropout-dl)

set(CMAKE_CXX_STANDARD 17)

add_executable(dropout-dl src/episode.cpp src/season.cpp src/series.cpp src/main.cpp)

target_link_libraries(dropout-dl curl)

find_library(SQLITE3_FOUND sqlite3)
find_library(GCRYPT_FOUND gcrypt)
IF (SQLITE3_FOUND)
    message(STATUS "sqlite3 found")
    target_link_libraries(dropout-dl curl sqlite3)
    add_compile_definitions(DROPOUT_DL_SQLITE)
ENDIF()
IF(GCRYPT_FOUND)
    message(STATUS "libgcrypt found")
    target_link_libraries(dropout-dl gcrypt)
    add_compile_definitions(DROPOUT_DL_GCRYPT)
ENDIF()

