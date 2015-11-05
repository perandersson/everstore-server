set(SOURCE_FILES main.cpp Store.cpp Store.h StoreClient.cpp StoreClient.h StoreServer.cpp StoreServer.h Authenticator.cpp Authenticator.h)


add_executable(everstore-server ${SOURCE_FILES})

## link with everstore-shared
set(EVERSTORE_LIB_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/everstore-shared")
set_property(TARGET everstore-server APPEND PROPERTY INCLUDE_DIRECTORIES ${EVERSTORE_LIB_INCLUDE_DIRS})
target_link_libraries(everstore-server everstore-shared)

set (OTHER_LIBS "")
if (MSVC)
	list(APPEND OTHER_LIBS ws2_32)
elseif(UNIX)
	list(APPEND OTHER_LIBS pthread)
	if(UNIX AND NOT APPLE)
		list(APPEND OTHER_LIBS rt)
	endif()
endif()

target_link_libraries(everstore-server ${OTHER_LIBS})
