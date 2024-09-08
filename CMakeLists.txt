cmake_minimum_required(VERSION 3.20)
project(Telemetry-Studio)

if(UNIX AND NOT APPLE)
	set(LINUX TRUE)

	set(IS_WIN32 0)
	set(IS_MACOS 0)
	set(IS_LINUX 1)
elseif(APPLE)
	set(MACOS TRUE)

	set(IS_WIN32 0)
	set(IS_MACOS 1)
	set(IS_LINUX 0)
else()
	set(IS_WIN32 1)
	set(IS_MACOS 0)
	set(IS_LINUX 0)
endif()

set(CMAKE_CXX_STANDARD 20)
cmake_policy(SET CMP0042 NEW)

add_subdirectory(source)
