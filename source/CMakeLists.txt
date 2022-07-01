cmake_minimum_required(VERSION 3.8)
project(tlm-studio)

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

# Qt crap
qt5_wrap_cpp(SOURCES
		model/generic_tree_model.h
		widgets/document_window.h
		widgets/time_picker_widget.h)
qt5_wrap_ui(SOURCES
		../resources/ui/document_window.ui)
qt5_add_resources(SOURCES
		../resources.qrc)

add_executable(tlm-studio WIN32 ${SOURCES})
set_target_properties(tlm-studio PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE})
target_sort_source_files(tlm-studio)

target_include_directories(tlm-studio SYSTEM PRIVATE ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR})

# Link and copy over Qt component DLLs
foreach(_component ${QtComponents})
	target_link_libraries(tlm-studio PRIVATE Qt5::${_component})
	add_custom_command(TARGET tlm-studio POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:Qt5::${_component}>" "$<TARGET_FILE_DIR:tlm-studio>")
endforeach()

# Copy over Qt resources
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
		add_custom_command(TARGET tlm-studio POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE_DIR:Qt5::Core>/../${_resource}" "$<TARGET_FILE_DIR:tlm-studio>")
	endif()
endforeach()

