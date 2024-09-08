cmake_minimum_required(VERSION 3.20)
project(tlm-viewer)

set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

set(QT_COMPONENTS
		Core
		Widgets
		Gui
		Qml
		QmlModels
		Quick
		QuickWidgets
		PrintSupport
		Network
		Positioning
		WebChannel
		WebEngineCore
		WebEngineWidgets)

find_package(Qt5 5.15.0 REQUIRED COMPONENTS ${QT_COMPONENTS})

list(TRANSFORM QT_COMPONENTS PREPEND "Qt5::")

set(SOURCES
		main.cpp
		model/generic_tree_item.cpp
		model/generic_tree_item.h
		model/generic_tree_model.cpp
		model/generic_tree_model.h
		model/performance_data.cpp
		model/performance_data.h
		model/recently_opened.cpp
		model/recently_opened.h
		model/telemetry_container.cpp
		model/telemetry_container.h
		model/telemetry_reader.cpp
		model/telemetry_reader.h
		utilities/color.h
		utilities/data_decimator.cpp
		utilities/data_decimator.h
		utilities/settings.cpp
		utilities/settings.h
		utilities/xplane_installations.cpp
		utilities/xplane_installations.h
		widgets/document_window.ui
		widgets/document_window.cpp
		widgets/document_window.h
		widgets/test_runner_dialog.ui
		widgets/test_runner_dialog.cpp
		widgets/test_runner_dialog.h
		widgets/chart_widget.cpp
		widgets/chart_widget.h
		widgets/time_picker_widget.cpp
		widgets/time_picker_widget.h
		widgets/timeline_widget.cpp
		widgets/timeline_widget.h
		../resources.qrc)

set(DEFINITIONS
	WIN=${IS_WIN32}
	LIN=${IS_LINUX}
	APL=${IS_MACOS})

add_executable(tlm-viewer ${SOURCES})

set_target_properties(tlm-viewer PROPERTIES WIN32_EXECUTABLE ON MACOSX_BUNDLE ON)
set_target_properties(tlm-viewer PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE})
set_target_properties(tlm-viewer PROPERTIES OUTPUT_NAME "Telemetry Viewer")

target_compile_definitions(tlm-viewer PRIVATE ${DEFINITIONS})
target_link_libraries(tlm-viewer ${QT_COMPONENTS})
target_include_directories(tlm-viewer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_custom_command(TARGET tlm-viewer POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory "$<TARGET_FILE_DIR:Qt5::Core>/../plugins/platforms" "$<TARGET_FILE_DIR:tlm-viewer>/platforms")

if(IS_WIN32)
	set(QT_RESOURCES
			$<$<CONFIG:Debug>:bin/QtWebEngineProcessd.exe>
			$<$<CONFIG:Debug>:resources/qtwebengine_devtools_resources.pak>
			$<$<NOT:$<CONFIG:Debug>>:bin/QtWebEngineProcess.exe>
			resources/icudtl.dat
			resources/qtwebengine_resources.pak
			resources/qtwebengine_resources_100p.pak
			resources/qtwebengine_resources_200p.pak)

	add_custom_target(copy-resources ALL)

	foreach(QT_COMPONENT ${QT_COMPONENTS})

		add_custom_command(
				TARGET copy-resources POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E copy_if_different
				$<TARGET_FILE:${QT_COMPONENT}>
				$<TARGET_FILE_DIR:tlm-viewer>
				COMMENT "Copying ${QT_COMPONENT} from $<TARGET_FILE:${QT_COMPONENT}> to $<TARGET_FILE_DIR:tlm-viewer>")

	endforeach()

	foreach(QT_RESOURCE ${QT_RESOURCES})

		add_custom_command(
				TARGET copy-resources POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E copy_if_different
				$<TARGET_FILE_DIR:Qt5::Core>/../${QT_RESOURCE}
				$<TARGET_FILE_DIR:tlm-viewer>
				COMMENT "Copying ${QT_RESOURCE} from $<TARGET_FILE_DIR:Qt5::Core>/../${QT_RESOURCE} to $<TARGET_FILE_DIR:tlm-viewer>")

	endforeach()

	add_dependencies(tlm-viewer copy-resources)
endif()

if(IS_LINUX)
	target_link_libraries(tlm-viewer PRIVATE stdc++ m)
endif()
