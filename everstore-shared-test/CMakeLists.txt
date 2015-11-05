set(SOURCE_FILES main.cpp Journal_test.cpp FileLock_test.cpp Transaction_test.cpp Timestamp_test.cpp Properties_test.cpp Bytes_test.cpp test/ITestCallback.h test/Test.h test/Test.cpp test/TestException.cpp test/TestException.h test/TestRunner.cpp test/TestRunner.h test/TestSuite.cpp test/TestSuite.h test/UnitTest.cpp test/UnitTest.h)
ENABLE_TESTING()

add_executable(everstore-shared-test ${SOURCE_FILES})

## link with everstore-shared
set(EVERSTORE_LIB_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/everstore-shared")
set_property(TARGET everstore-shared-test APPEND PROPERTY INCLUDE_DIRECTORIES ${EVERSTORE_LIB_INCLUDE_DIRS})
target_link_libraries(everstore-shared-test everstore-shared)

if(MSVC)
	target_link_libraries(everstore-shared-test ws2_32)
endif()

## Copy test-resources into bin test-directory
file(COPY test-resources DESTINATION "${EXECUTABLE_OUTPUT_PATH}/")