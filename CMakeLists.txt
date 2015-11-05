cmake_minimum_required(VERSION 3.2)
project(everstore)

# Default configuration types
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")

# Add configuration for MSVC builds
IF(MSVC)
	ADD_DEFINITIONS(/fp:fast)
	ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS=1)
	ADD_DEFINITIONS(-Deverstore_BUILD=1)
ELSEIF(APPLE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -stdlib=libc++ -std=c++11 -Wno-invalid-offsetof")
ELSE()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pthread -std=c++11 -Wno-invalid-offsetof")
ENDIF()

IF(everstore_BUILD_STATIC)
	IF(MSVC)
		ADD_DEFINITIONS(-Deverstore_STATIC=1)
	ENDIF()
ENDIF()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/bin")
set(LIBRARY_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/bin")

add_subdirectory (everstore-shared)
add_subdirectory (everstore-shared-test)
add_subdirectory (everstore-worker)
add_subdirectory (everstore-server)