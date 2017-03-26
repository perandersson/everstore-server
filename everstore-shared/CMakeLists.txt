file(GLOB all_files *.cpp *.h)

if (WIN32)
    file(GLOB all_socket_files win32/*.cpp win32/*.h)
else ()
    file(GLOB all_socket_files gcc/*.cpp gcc/*.h)
endif ()

add_library(everstore-shared ${all_files} ${all_socket_files})
