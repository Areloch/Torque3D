project(${TORQUE_APP_NAME})

# Enable ObjectiveC compilation when compiling for Apple platforms
if (APPLE)
	enable_language(OBJC)
endif (APPLE)

set(TORQUE_SDL ON) # we need sdl to do our platform interop
forwardDef(TORQUE_SDL)

# Linux requires X11 & freetype
if (UNIX AND NOT APPLE)
	set(TORQUE_SOURCE_FILES ${TORQUE_SOURCE_FILES} ${TORQUE_PLATFORM_X11_SOURCES})  
	find_package(Freetype REQUIRED)
	set(TORQUE_INCLUDE_DIRECTORIES ${TORQUE_INCLUDE_DIRECTORIES} ${FREETYPE_INCLUDE_DIRS})
endif (UNIX AND NOT APPLE)


################# Engine Module Handling ###################

set(TORQUE_MODULE_PATHS "${CMAKE_SOURCE_DIR}/Tools/CMake/modules" "${TORQUE_APP_GAME_DIRECTORY}/data")
if (NOT "${TORQUE_MODULE_USER_PATH}" STREQUAL "")
	list(APPEND TORQUE_MODULE_PATHS "${TORQUE_MODULE_USER_PATH}")
endif()

# Before doing module scanning, store away the engine sources - we do this so that modules
# can be placed into the proper filters
set(TORQUE_SOURCE_FILES_TEMPORARY ${TORQUE_SOURCE_FILES})
set(TORQUE_SOURCE_FILES "")

foreach (TORQUE_MODULE_PATH ${TORQUE_MODULE_PATHS})
	# First find simple cmake scripts, mostly used for in-engine modules
	file(GLOB MODULE_SCRIPTS "${TORQUE_MODULE_PATH}/*.cmake")
	foreach (TORQUE_MODULE_SCRIPT ${MODULE_SCRIPTS})
		include(${TORQUE_MODULE_SCRIPT})

        # Add this script's collected files to our Engine group
        source_group(TREE "${CMAKE_SOURCE_DIR}/Engine" PREFIX "Engine" FILES ${TORQUE_SOURCE_FILES})

        set(TORQUE_SOURCE_FILES_TEMPORARY ${TORQUE_SOURCE_FILES_TEMPORARY} ${TORQUE_SOURCE_FILES})
        set(TORQUE_SOURCE_FILES "")
	endforeach()

	# Next find sub projects, these can introduce new source files
	SUBDIRLIST(POSSIBLE_PROJECTS "${TORQUE_MODULE_PATH}")
	foreach (POSSIBLE_PROJECT ${POSSIBLE_PROJECTS})
		# Retrieve the absolute path of this possible project
		get_filename_component(POSSIBLE_PROJECT_ABSOLUTEPATH "${POSSIBLE_PROJECT}"
							   REALPATH BASE_DIR "${TORQUE_MODULE_PATH}")

		if (EXISTS "${POSSIBLE_PROJECT_ABSOLUTEPATH}/CMakeLists.txt")
			add_subdirectory("${POSSIBLE_PROJECT_ABSOLUTEPATH}" ${CMAKE_BINARY_DIR}/temp/${POSSIBLE_PROJECT} EXCLUDE_FROM_ALL)
            source_group(TREE "${POSSIBLE_PROJECT_ABSOLUTEPATH}" PREFIX "Modules/${POSSIBLE_PROJECT}" FILES ${TORQUE_SOURCE_FILES})

            set(TORQUE_SOURCE_FILES_TEMPORARY ${TORQUE_SOURCE_FILES_TEMPORARY} ${TORQUE_SOURCE_FILES})
            set(TORQUE_SOURCE_FILES "")
        elseif(NOT EXISTS "${POSSIBLE_PROJECT_ABSOLUTEPATH}/*.cmake")
            file(GLOB_RECURSE MODULE_SOURCE "${POSSIBLE_PROJECT_ABSOLUTEPATH}/source/*.h" "${POSSIBLE_PROJECT_ABSOLUTEPATH}/source/*.cpp")
            #message(STATUS "MODULE_SOURCE:${MODULE_SOURCE}")
            source_group(TREE "${POSSIBLE_PROJECT_ABSOLUTEPATH}" PREFIX "Modules/${POSSIBLE_PROJECT}/" FILES ${MODULE_SOURCE})
            set(TORQUE_SOURCE_FILES_TEMPORARY ${TORQUE_SOURCE_FILES_TEMPORARY} ${MODULE_SOURCE})
		endif()
	endforeach()
endforeach()

set(TORQUE_SOURCE_FILES ${TORQUE_SOURCE_FILES_TEMPORARY})

################# Library Post-build Handling ###################
set(TORQUE_LIBRARY_PATHS "${CMAKE_SOURCE_DIR}/Engine/lib" "${TORQUE_APP_GAME_DIRECTORY}/data")
if (NOT "${TORQUE_LIBRARY_USER_PATH}" STREQUAL "")
	list(APPEND TORQUE_LIBRARY_PATHS "${TORQUE_LIBRARY_USER_PATH}")
endif()

foreach (TORQUE_LIBRARY_PATH ${TORQUE_LIBRARY_PATHS})
	# First find simple cmake scripts, mostly used for in-engine libraries
	file(GLOB_RECURSE LIBRARY_SCRIPTS "${TORQUE_LIBRARY_PATH}/*Torque_postBuild.cmake")
    #message(STATUS "LIBRARY_SCRIPTS:${LIBRARY_SCRIPTS}")
	foreach (TORQUE_LIBRARY_SCRIPT ${LIBRARY_SCRIPTS})
		include(${TORQUE_LIBRARY_SCRIPT})
	endforeach()
endforeach()

#target_compile_definitions(${TORQUE_APP_NAME} PUBLIC ${TORQUE_COMPILE_DEFINITIONS})
#target_link_libraries(${TORQUE_APP_NAME} ${TORQUE_LINK_LIBRARIES})
#target_include_directories(${TORQUE_APP_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} "${CMAKE_BINARY_DIR}/temp" ${TORQUE_INCLUDE_DIRECTORIES})
#target_compile_features(${TORQUE_APP_NAME} PRIVATE cxx_std_20)
append_defs()

# Process library binaries - these are coming from modules that are providing links to external, precompiled code that should be included
# with the executable. This is done because on Windows, the .lib is separate from the .dll so we can't automatically scan for shared
# objects in our link libraries in that case.
foreach (LIBRARY_BINARY ${TORQUE_ADDITIONAL_LIBRARY_BINARIES})
  if (APPLE)
    # For OSX, we want these binaries to be copied to the Frameworks directory
    add_custom_command(TARGET ${TORQUE_APP_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${LIBRARY_BINARY} "${TORQUE_APP_GAME_DIRECTORY}/${TORQUE_APP_NAME}.app/Contents/Frameworks")
  else()
    # All other platforms expect the file next to the executable
    add_custom_command(TARGET ${TORQUE_APP_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${LIBRARY_BINARY} "${TORQUE_APP_GAME_DIRECTORY}")
  endif (APPLE)
endforeach()

# Process link libraries for dynamic links - we do this on OSX/Linux to ensure the binaries end up in the correct App directory
# as in the root CMake we force everything to be in game. This is necessary because on these platforms these are considered "libraries"
# and not runtime binaries like we configure in the root CMake. We don't globally set library outputs to avoid outputting eg. a files to
# our game directory.
if (UNIX)
	get_target_property(GAME_LINK_LIBRARIES ${TORQUE_APP_NAME} LINK_LIBRARIES)
	foreach (GAME_LINK_LIBRARY ${GAME_LINK_LIBRARIES})
	  # For eg. OSX some links are not valid targets - for example frameworks provided by OS
	  if (TARGET ${GAME_LINK_LIBRARY})
		  get_target_property(LINK_LIBRARY_TYPE ${GAME_LINK_LIBRARY} TYPE)
			
		  # Only pay attention to shared libraries and make them output to the app resources
		  if ("${LINK_LIBRARY_TYPE}" STREQUAL "SHARED_LIBRARY")	 
        if (APPLE)
          set_target_properties(${GAME_LINK_LIBRARY} PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${TORQUE_APP_GAME_DIRECTORY}/${TORQUE_APP_NAME}.app/Contents/Frameworks")
        else()
          set_target_properties(${GAME_LINK_LIBRARY} PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${TORQUE_APP_GAME_DIRECTORY}")
        endif(APPLE)
		  endif()
	  endif()
	endforeach()
endif (UNIX)