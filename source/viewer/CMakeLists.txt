cmake_minimum_required(VERSION 3.20)
project(Telemetry-Viewer)

set(VIEWER_VERSION_MAJOR 0)
set(VIEWER_VERSION_MINOR 6)
set(VIEWER_VERSION_PATCH 0)

set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

set(QT_VERSION 6.8.2)

set(QT_COMPONENTS
		Core
		Widgets
		Gui
		Charts)

set(QT_OPTIONAL_COMPONENTS
		OpenGL
		OpenGLWidgets)

find_package(Qt6 ${QT_VERSION} REQUIRED COMPONENTS ${QT_COMPONENTS})

foreach(COMPONENT ${QT_OPTIONAL_COMPONENTS})
	find_package(Qt6 ${QT_VERSION} QUIET COMPONENTS ${COMPONENT})

	if(Qt6${COMPONENT}_FOUND)
		list(APPEND QT_COMPONENTS ${COMPONENT})
	endif()

endforeach()

list(TRANSFORM QT_COMPONENTS PREPEND "Qt6::")

set(QT_PLATFORMS_DIR "${Qt6_DIR}/../../../plugins/platforms")

set(SOURCES
		main.cpp
		Application.cpp
		model/TelemetryDocument.cpp
		model/TelemetryDocument.h
		model/XplaneInstallation.cpp
		model/XplaneInstallation.h
		utilities/Color.h
		utilities/DataDecimator.cpp
		utilities/DataDecimator.h
		utilities/PerformanceCalculator.cpp
		utilities/PerformanceCalculator.h
		utilities/RunningAverage.h
		utilities/Settings.cpp
		utilities/Settings.h
		widgets/ChartCallout.cpp
		widgets/ChartCallout.h
		widgets/ChartWidget.cpp
		widgets/ChartWidget.h
		widgets/DocumentWindow.ui
		widgets/DocumentWindow.cpp
		widgets/DocumentWindow.h
		widgets/TestRunnerDialog.ui
		widgets/TestRunnerDialog.cpp
		widgets/TestRunnerDialog.h
		widgets/TimePickerWidget.cpp
		widgets/TimePickerWidget.h
		widgets/TimelineWidget.cpp
		widgets/TimelineWidget.h
		resources/resources.qrc)

set(DEFINITIONS
		WIN=${IS_WIN32}
		LIN=${IS_LINUX}
		APL=${IS_MACOS}
		VERSION_MAJOR=${VIEWER_VERSION_MAJOR}
		VERSION_MINOR=${VIEWER_VERSION_MINOR}
		VERSION_PATCH=${VIEWER_VERSION_PATCH})

add_executable(tlm-viewer WIN32 MACOSX_BUNDLE ${SOURCES})
set_target_properties(tlm-viewer PROPERTIES OUTPUT_NAME "Telemetry Viewer")

target_link_libraries(tlm-viewer tlm-static ${QT_COMPONENTS})
target_compile_definitions(tlm-viewer PRIVATE ${DEFINITIONS})
target_include_directories(tlm-viewer SYSTEM PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(IS_WIN32)

	set(QT_PLATFORM_PLUGIN "${QT_PLATFORMS_DIR}/qwindows$<$<CONFIG:Debug>:d>.dll")

	add_custom_target(tlm-viewer-resources ALL)

	add_custom_command(TARGET tlm-viewer-resources POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:tlm-viewer>/platforms/"
			COMMAND ${CMAKE_COMMAND} -E copy_if_different "${QT_PLATFORM_PLUGIN}" "$<TARGET_FILE_DIR:tlm-viewer>/platforms/")

	install(FILES "${QT_PLATFORM_PLUGIN}" DESTINATION ${CMAKE_INSTALL_BINDIR}/platforms)

	foreach(QT_COMPONENT ${QT_COMPONENTS})

		add_custom_command(TARGET tlm-viewer-resources POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:${QT_COMPONENT}>" "$<TARGET_FILE_DIR:tlm-viewer>")

		install(FILES "$<TARGET_FILE:${QT_COMPONENT}>" DESTINATION ${CMAKE_INSTALL_BINDIR})

	endforeach()

	add_dependencies(tlm-viewer tlm-viewer-resources)
	install(FILES $<TARGET_PDB_FILE:tlm-viewer> DESTINATION ${CMAKE_INSTALL_BINDIR} OPTIONAL)

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

		install(FILES $<TARGET_FILE:${QT_COMPONENT}> DESTINATION ${CMAKE_INSTALL_LIBDIR})

	endforeach()

endif()

install(TARGETS tlm-viewer
		BUNDLE DESTINATION ${CMAKE_INSTALL_BINDIR})
