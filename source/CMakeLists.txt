cmake_minimum_required(VERSION 3.20)
project(Telemetry-Viewer)

set(VIEWER_VERSION_MAJOR 0)
set(VIEWER_VERSION_MINOR 5)
set(VIEWER_VERSION_PATCH 0)

set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

set(QT_COMPONENTS
		Core
		Widgets
		Gui
		Charts
		OpenGL
		OpenGLWidgets)

find_package(Qt6 6.7.0 REQUIRED COMPONENTS ${QT_COMPONENTS})

list(TRANSFORM QT_COMPONENTS PREPEND "Qt6::")

set(QT_PLATFORMS_DIR "${Qt6_DIR}/../../../plugins/platforms")

set(SOURCES
		main.cpp
		model/generic_tree_item.cpp
		model/generic_tree_item.h
		model/generic_tree_model.cpp
		model/generic_tree_model.h
		model/recently_opened.cpp
		model/recently_opened.h
		model/telemetry_container.cpp
		model/telemetry_container.h
		model/telemetry_reader.cpp
		model/telemetry_reader.h
		utilities/color.h
		utilities/data_decimator.cpp
		utilities/data_decimator.h
		utilities/performance_calculator.cpp
		utilities/performance_calculator.h
		utilities/providers.h
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
	APL=${IS_MACOS}
	VERSION_MAJOR=${VIEWER_VERSION_MAJOR}
	VERSION_MINOR=${VIEWER_VERSION_MINOR}
	VERSION_PATCH=${VIEWER_VERSION_PATCH})

add_executable(tlm-viewer WIN32 MACOSX_BUNDLE ${SOURCES})
set_target_properties(tlm-viewer PROPERTIES OUTPUT_NAME "Telemetry Viewer")

target_link_libraries(tlm-viewer ${QT_COMPONENTS})
target_compile_definitions(tlm-viewer PRIVATE ${DEFINITIONS})
target_include_directories(tlm-viewer SYSTEM PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(IS_WIN32)

	set(QT_PLATFORM_PLUGIN "${QT_PLATFORMS_DIR}/qwindows$<$<CONFIG:Debug>:d>.dll")

	add_custom_target(tlm-viewer-resources ALL)

	add_custom_command(TARGET tlm-viewer-resources POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:tlm-viewer>/platforms/"
			COMMAND ${CMAKE_COMMAND} -E copy_if_different "${QT_PLATFORM_PLUGIN}" "$<TARGET_FILE_DIR:tlm-viewer>/platforms/")

	install(FILES "${QT_PLATFORM_PLUGIN}" DESTINATION bin/platforms)

	foreach(QT_COMPONENT ${QT_COMPONENTS})

		add_custom_command(TARGET tlm-viewer-resources POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:${QT_COMPONENT}>" "$<TARGET_FILE_DIR:tlm-viewer>")

		install(FILES "$<TARGET_FILE:${QT_COMPONENT}>" DESTINATION bin)

	endforeach()

	add_dependencies(tlm-viewer tlm-viewer-resources)

endif()

if(IS_MACOS)
	add_custom_target(tlm-viewer-resources ALL)

	set(QT_PLATFORM_PLUGIN "${QT_PLATFORMS_DIR}/libqcocoa.dylib")

	add_custom_command(TARGET tlm-viewer-resources POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_BUNDLE_DIR:tlm-viewer>/Contents/Frameworks"
			COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_BUNDLE_DIR:tlm-viewer>/Contents/plugins/platforms/"
			COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_BUNDLE_DIR:tlm-viewer>/Contents/plugins/platforms/../../lib")

	add_custom_command(
			TARGET tlm-viewer-resources POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different "${QT_PLATFORM_PLUGIN}" "$<TARGET_BUNDLE_DIR:tlm-viewer>/Contents/plugins/platforms/")

	add_dependencies(tlm-viewer tlm-viewer-resources)

	install(CODE "
        include(BundleUtilities)
        fixup_bundle(\"$<TARGET_BUNDLE_DIR:tlm-viewer>\" \"\" \"\")
        execute_process(
            COMMAND install_name_tool -change @rpath/QtGui.framework/Versions/A/QtGui @rpath/../Frameworks/QtGui.framework/Versions/A/QtGui \"$<TARGET_BUNDLE_DIR:tlm-viewer>/Contents/plugins/platforms/libqcocoa.dylib\")
        execute_process(
            COMMAND install_name_tool -change @rpath/QtCore.framework/Versions/A/QtCore @rpath/../Frameworks/QtCore.framework/Versions/A/QtCore \"$<TARGET_BUNDLE_DIR:tlm-viewer>/Contents/plugins/platforms/libqcocoa.dylib\")
        ")
endif()

if(IS_LINUX)
	target_link_libraries(tlm-viewer stdc++ m)

	foreach(QT_COMPONENT ${QT_COMPONENTS})

		install(FILES $<TARGET_FILE:${QT_COMPONENT}> DESTINATION lib)

	endforeach()

endif()

install(TARGETS tlm-viewer
		BUNDLE DESTINATION bin)
