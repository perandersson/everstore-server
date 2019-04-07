cmake_minimum_required(VERSION 3.6)
project(everstore)
set(CMAKE_CXX_STANDARD 11)

# Default configuration types
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")

# Collect all server files
file(GLOB ALL_SERVER_FILES Server/*.cpp Server/*.h Server/Auth/*.* Server/Ipc/*.*)

# Collect all shared files
file(GLOB ALL_SHARED_FILES Shared/*.cpp Shared/*.h
        Shared/File/*.* Shared/Memory/*.* Shared/Database/*.*
        Shared/Ipc/*.* Shared/Message/*.* Shared/Network/*.* Shared/Log/*.*
        Shared/Mutex/*.* Shared/Process/*.* Shared/Socket/*.*)
if (WIN32)
    file(GLOB ALL_OS_SHARED_FILES Shared/Network/Win32/*.* Shared/Mutex/Win32/*.*
            Shared/Process/Win32/*.* Shared/Socket/Win32/*.*)
elseif (APPLE)
    file(GLOB ALL_OS_SHARED_FILES Shared/Network/Unix/*.* Shared/Mutex/Unix/*.*
            Shared/Process/Unix/*.* Shared/Socket/Unix/*.*)
else ()
    file(GLOB ALL_OS_SHARED_FILES Shared/Network/Unix/*.* Shared/Mutex/Unix/*.*
            Shared/Process/Unix/*.* Shared/Socket/Unix/*.*)
endif ()

# Collect all worker files
FILE(GLOB ALL_WORKER_FILES Worker/*.cpp Worker/*.h)

# Collect all test files
FILE(GLOB ALL_TEST_FILES Test/*.cpp Test/*.h Test/test/*.*)

# Default Microsoft Windows compiler properties
set(MSVC_DEFINITIONS "/fp:fast /W4 /D_CRT_SECURE_NO_WARNINGS=1 /wd4201 /wd4100 /D_WIN32_WINNT=0x0602")
set(MSVC_SPECIFIL_LIBS "legacy_stdio_definitions.lib")

# Default GCC for Linux and GCC for OSX compiler properties
# TODO: dynamically link libraries for now. -static linking causes "segmentation fault" when using shm_open in worker thread.
set(GCC_DEFINITIONS "-Wno-unused-const-variable -Wno-unused-variable -funsafe-math-optimizations -Wall -std=c++11 -g -rdynamic")
set(OSX_GCC_DEFINITIONS "-Wno-unused-const-variable -Wno-unused-variable -funsafe-math-optimizations -Wall -std=c++11 -stdlib=libc++")

# Configure libs
if (WIN32)
    set(OS_SPECIFIC_LIBS ${MSVC_SPECIFIL_LIBS} "ws2_32")
elseif (APPLE)
    set(OS_SPECIFIC_LIBS "pthread")
else ()
    set(OS_SPECIFIC_LIBS "rt" "pthread")
endif ()

# Add configuration for the various compilers
if (MSVC)
    ADD_DEFINITIONS(${MSVC_DEFINITIONS})
elseif (APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OSX_GCC_DEFINITIONS}")
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_DEFINITIONS}")
endif ()

# Server executable
add_executable(everstore-server ${ALL_SERVER_FILES} ${ALL_SHARED_FILES} ${ALL_OS_SHARED_FILES})
target_link_libraries(everstore-server ${OS_SPECIFIC_LIBS})
if (LINUX AND NOT APPLE)
    set_target_properties(everstore-server PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
endif ()

# Server executable
add_executable(everstore-worker ${ALL_WORKER_FILES} ${ALL_SHARED_FILES} ${ALL_OS_SHARED_FILES})
target_link_libraries(everstore-worker ${OS_SPECIFIC_LIBS})
if (LINUX AND NOT APPLE)
    set_target_properties(everstore-worker PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
endif ()

# Server tests
add_executable(everstore-tests ${ALL_TEST_FILES} ${ALL_SHARED_FILES} ${ALL_OS_SHARED_FILES})
target_link_libraries(everstore-tests ${OS_SPECIFIC_LIBS})
if (LINUX AND NOT APPLE)
    set_target_properties(everstore-tests PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
endif ()

# If we are running on a platform NOT windows then set the output directory to "bin" so that
# we can know, beforehand, that we have the necessary permissions
if (NOT WIN32)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
    set(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/bin")
    set(LIBRARY_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/bin")
endif ()

## Copy test-resources into bin test-directory
file(COPY Test/test-resources DESTINATION "${EXECUTABLE_OUTPUT_PATH}/")
