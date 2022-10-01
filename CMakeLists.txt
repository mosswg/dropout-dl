cmake_minimum_required(VERSION 3.23)
project(dropout-dl)

set(CMAKE_CXX_STANDARD 17)


IF(DROPOUT_DL_BUILD_ALL)
    add_executable(dropout-dl-minimal src/episode.cpp src/season.cpp src/series.cpp src/main.cpp)

    target_link_libraries(dropout-dl-minimal curl)


    add_executable(dropout-dl-sqlite src/episode.cpp src/season.cpp src/series.cpp src/main.cpp)

    target_link_libraries(dropout-dl-sqlite curl sqlite3)

    target_compile_definitions(dropout-dl-sqlite PUBLIC DROPOUT_DL_SQLITE)


    add_executable(dropout-dl-full src/episode.cpp src/season.cpp src/series.cpp src/main.cpp)

    target_link_libraries(dropout-dl-full curl gcrypt sqlite3)

    target_compile_definitions(dropout-dl-full PUBLIC DROPOUT_DL_SQLITE DROPOUT_DL_GCRYPT)


    add_subdirectory(tests)
ELSE()

add_executable(dropout-dl src/episode.cpp src/season.cpp src/series.cpp src/main.cpp)

target_link_libraries(dropout-dl curl)

find_library(SQLITE3_FOUND sqlite3)
find_library(GCRYPT_FOUND gcrypt)
IF (SQLITE3_FOUND)
    message(STATUS "sqlite3 found")
    target_link_libraries(dropout-dl sqlite3)
    add_compile_definitions(DROPOUT_DL_SQLITE)
ENDIF()
IF(GCRYPT_FOUND)
    message(STATUS "libgcrypt found")
    target_link_libraries(dropout-dl gcrypt)
    add_compile_definitions(DROPOUT_DL_GCRYPT)
ENDIF()
IF(DROPOUT_DL_TESTS)
    add_subdirectory(tests)
ENDIF()

ENDIF()