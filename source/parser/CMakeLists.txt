cmake_minimum_required(VERSION 3.20)
project(Telemetry-Library)

set(SOURCES
		telemetry/container.cpp
		telemetry/event.cpp
		telemetry/parser.cpp
		telemetry/provider.cpp
		telemetry/statistic.cpp)

set(PUBLIC_HEADERS
		telemetry/container.h
		telemetry/data.h
		telemetry/event.h
		telemetry/parser.h
		telemetry/provider.h
		telemetry/statistic.h)

add_library(tlm SHARED ${SOURCES} ${PUBLIC_HEADERS})
target_include_directories(tlm PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(tlm-static STATIC ${SOURCES} ${PUBLIC_HEADERS})
target_include_directories(tlm-static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS tlm tlm-static DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${PUBLIC_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/telemetry)

if(IS_WIN32)
	install(FILES $<TARGET_PDB_FILE:tlm> DESTINATION ${CMAKE_INSTALL_LIBDIR} OPTIONAL)
endif()
