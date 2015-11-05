set(SOURCE_FILES main.cpp Worker.cpp Worker.h AttachedSockets.cpp AttachedSockets.h Journals.cpp Journals.h)


add_executable(everstore-worker ${SOURCE_FILES})

## link with everstore-shared
set(EVERSTORE_LIB_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/everstore-shared")
set_property(TARGET everstore-worker APPEND PROPERTY INCLUDE_DIRECTORIES ${EVERSTORE_LIB_INCLUDE_DIRS})
target_link_libraries(everstore-worker everstore-shared)

set (OTHER_LIBS "")
if (MSVC)
	list(APPEND OTHER_LIBS ws2_32)
elseif(UNIX)
	list(APPEND OTHER_LIBS pthread)
	if(UNIX AND NOT APPLE)
		list(APPEND OTHER_LIBS rt)
	endif()
endif()

target_link_libraries(everstore-worker ${OTHER_LIBS})
