cmake_minimum_required(VERSION 3.8)
project(Telemetry-Studio)

# TODO: Dynamic look up of Qt
if(APPLE)
	set(CMAKE_PREFIX_PATH "~/Qt/5.11.0/clang_64")
else()
	set(CMAKE_PREFIX_PATH "C:/Qt/5.15.0/msvc2019_64")
endif()

set(CMAKE_CXX_STANDARD 11)

cmake_policy(SET CMP0042 NEW)


include(cmake/target.cmake)

add_subdirectory(source)
