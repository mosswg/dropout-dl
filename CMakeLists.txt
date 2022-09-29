cmake_minimum_required(VERSION 3.23)
project(dropout-dl)

set(CMAKE_CXX_STANDARD 17)

add_executable(dropout-dl src/episode.cpp src/season.cpp src/series.cpp src/main.cpp)

IF (EXISTS "firefox_profile")
    target_link_libraries(dropout-dl curl sqlite3)
    add_compile_definitions(DROPOUT_DL_SQLITE)
ELSEIF(EXISTS "chrome_profile")
    target_link_libraries(dropout-dl curl sqlite3 gcrypt)
    add_compile_definitions(DROPOUT_DL_SQLITE DROPOUT_DL_GCRYPT)
ELSE()
    target_link_libraries(dropout-dl curl)
ENDIF()

