cmake_minimum_required(VERSION 3.20)
project(Telemetry-Library)

set(SOURCES
        telemetry/container.cpp
        telemetry/container.h
        telemetry/parser.cpp
        telemetry/parser.h
        telemetry/provider.cpp
        telemetry/provider.h
        telemetry/data.h
        telemetry/statistic.cpp
        telemetry/statistic.h
        telemetry/event.cpp
        telemetry/event.h)

add_library(tlm STATIC ${SOURCES})
target_include_directories(tlm PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS tlm)
