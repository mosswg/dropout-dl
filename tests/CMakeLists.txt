cmake_minimum_required(VERSION 3.23)
project(dropout-dl-tests)

set(CMAKE_CXX_STANDARD 17)

add_executable(test
	../src/util.cpp
	test.cpp
	episode_tests.cpp
	../src/episode.cpp
	../src/season.cpp
	series_tests.cpp
	../src/series.cpp
	season_tests.cpp
	)

target_link_libraries(test stdc++fs)

include_directories(../external/json/include)

target_link_libraries(test nlohmann_json::nlohmann_json)

target_link_libraries(test curl sqlite3 gcrypt)

message(STATUS "Building Tests")
