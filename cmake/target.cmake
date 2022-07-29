
macro(target_sort_source_files __TARGET)

	set(SOURCE_GROUP_DELIMITER "/")
	get_target_property(__SOURCES ${__TARGET} SOURCES)

	foreach(__FILE ${__SOURCES})

		if(EXISTS ${__FILE})
			cmake_path(RELATIVE_PATH __FILE OUTPUT_VARIABLE __RELATIVE_PATH)
			get_filename_component(__DIRECTORY "${__RELATIVE_PATH}" PATH)
		else()
			set(__ABSOLUTE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/${__FILE})
			cmake_path(RELATIVE_PATH __ABSOLUTE_PATH OUTPUT_VARIABLE __RELATIVE_PATH)
			get_filename_component(__DIRECTORY "${__RELATIVE_PATH}" PATH)
		endif()

		if(NOT ("${__DIRECTORY}" STREQUAL "${__LAST_DIRECTORY}"))
			if(__FILES)
				source_group("${__LAST_DIRECTORY}" FILES ${__FILES})
			endif()

			set(__LAST_DIRECTORY "${__DIRECTORY}")
			set(__FILES "")
		endif()

		list(APPEND __FILES ${__FILE})


	endforeach()

	if(__FILES)
		source_group("${__LAST_DIRECTORY}" FILES ${__FILES})
	endif()

endmacro()

macro(add_header_files __SOURCE_VAR)

	set(__SOURCES ${${__SOURCE_VAR}})
	set(__HEADERS "")

	foreach(__FILE ${__SOURCES})

		get_filename_component(__EXT "${__FILE}" EXT)

		if("${__EXT}" MATCHES "(c|cpp|cxx)")

			get_filename_component(__DIRECTORY "${__FILE}" PATH)
			get_filename_component(__NAME "${__FILE}" NAME_WE)

			get_filename_component(__ABSOLUTE "${__DIRECTORY}/${__NAME}" ABSOLUTE)


			if(NOT EXISTS "${__ABSOLUTE}${__EXT}")
				get_filename_component(__ABSOLUTE "${CMAKE_CURRENT_SOURCE_DIR}/${__ABSOLUTE}" ABSOLUTE)
			endif()


			if(EXISTS "${__ABSOLUTE}.h")

				file(RELATIVE_PATH __RELATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}" "${__ABSOLUTE}.h")
				list(APPEND __HEADERS ${__RELATIVE_PATH})

			endif ()

			if(EXISTS "${__ABSOLUTE}.hpp")

				file(RELATIVE_PATH __RELATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}" "${__ABSOLUTE}.hpp")
				list(APPEND __HEADERS ${__RELATIVE_PATH})

			endif ()

		endif()

	endforeach()


	list(APPEND ${__SOURCE_VAR} ${__HEADERS})

endmacro()

function(target_set_name TARGET NAME)
	set_target_properties(${__TARGET} PROPERTIES OUTPUT_NAME "${__NAME}")

	foreach(__CONFIG ${CMAKE_CONFIGURATION_TYPES})
		set_target_properties(${__TARGET} PROPERTIES OUTPUT_NAME_${__CONFIG} "${__NAME}")
	endforeach()
endfunction()
