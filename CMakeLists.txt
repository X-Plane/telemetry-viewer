cmake_minimum_required(VERSION 3.20)
project(X-Telemetry-Utils)

include(GNUInstallDirs)

if(UNIX AND NOT APPLE)
	set(IS_WIN32 0)
	set(IS_MACOS 0)
	set(IS_LINUX 1)
elseif(APPLE)
	set(IS_WIN32 0)
	set(IS_MACOS 1)
	set(IS_LINUX 0)

	set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64")
else()
	set(IS_WIN32 1)
	set(IS_MACOS 0)
	set(IS_LINUX 0)

	add_compile_options(/MP)
endif()

set(CMAKE_CXX_STANDARD 20)

cmake_policy(SET CMP0042 NEW)

add_subdirectory(source)
