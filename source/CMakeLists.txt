cmake_minimum_required(VERSION 3.20)
project(tlm-viewer)

set(CMAKE_AUTORCC ON)

set(QtComponents
		Core
		Widgets
		Gui
		Charts
		WebEngineWidgets
		WebEngineCore
		WebChannel
		PrintSupport
		QuickWidgets
		Network
		Positioning
		Quick
		Qml
		QmlModels)

find_package(Qt5 COMPONENTS ${QtComponents} REQUIRED)

set(SOURCES
		main.cpp
		model/generic_tree_item.cpp
		model/generic_tree_model.cpp
		model/telemetry_container.cpp
		model/telemetry_reader.cpp
		utilities/data_decimator.cpp
		utilities/color.h
		widgets/document_window.cpp
		widgets/chart_widget.cpp
		widgets/time_picker_widget.cpp)

add_header_files(SOURCES)

set(DEFINITIONS
	WIN=${IS_WIN32}
	LIN=${IS_LINUX}
	APL=${IS_MACOS})

# Qt crap
qt5_wrap_cpp(SOURCES
		model/generic_tree_model.h
		widgets/document_window.h
		widgets/time_picker_widget.h)
qt5_wrap_ui(SOURCES
		../resources/ui/document_window.ui)
qt5_add_resources(SOURCES
		../resources.qrc)

add_executable(tlm-viewer ${SOURCES})
target_sort_source_files(tlm-viewer)

set_target_properties(tlm-viewer PROPERTIES WIN32_EXECUTABLE ON MACOSX_BUNDLE ON)
set_target_properties(tlm-viewer PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE})
set_target_properties(tlm-viewer PROPERTIES OUTPUT_NAME "Telemetry Viewer")

target_compile_definitions(tlm-viewer PRIVATE ${DEFINITIONS})
target_include_directories(tlm-viewer SYSTEM PRIVATE ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR})
target_include_directories(tlm-viewer PRIVATE ${PROJECT_SOURCE_DIR}/widgets)

# Link and copy over Qt component DLLs
foreach(_component ${QtComponents})
	target_link_libraries(tlm-viewer PRIVATE Qt5::${_component})
	add_custom_command(TARGET tlm-viewer POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:Qt5::${_component}>" "$<TARGET_FILE_DIR:tlm-viewer>")
endforeach()

# Copy over Qt resources
if(WIN32)
	set(QtResources
			$<$<CONFIG:Debug>:bin/QtWebEngineProcessd.exe>
			$<$<CONFIG:Debug>:resources/qtwebengine_devtools_resources.pak>
			$<$<NOT:$<CONFIG:Debug>>:bin/QtWebEngineProcess.exe>
			resources/icudtl.dat
			resources/qtwebengine_resources.pak
			resources/qtwebengine_resources_100p.pak
			resources/qtwebengine_resources_200p.pak)

	foreach(_resource ${QtResources})
		if(NOT "${_resource}" STREQUAL "")
			add_custom_command(TARGET tlm-viewer POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE_DIR:Qt5::Core>/../${_resource}" "$<TARGET_FILE_DIR:tlm-viewer>")
		endif()
	endforeach()

	# Copy the platform data
	add_custom_command(TARGET tlm-viewer POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory "$<TARGET_FILE_DIR:Qt5::Core>/../plugins/platforms" "$<TARGET_FILE_DIR:tlm-viewer>/platforms")
endif()
