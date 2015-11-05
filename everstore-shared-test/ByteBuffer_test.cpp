#include <everstore.h>
#include "test/Test.h"

TEST_SUITE(ByteBuffer) {

	UNIT_TEST(defaultSize) {
		ByteBuffer b;

		ASSERT_EQUALS(b.size(), 0);
		
		const char* start = b.buffer(0);
		const char* end = b.buffer(UINT32_MAX);
		const uint32_t size = (uint32_t)(end - start) + 1;
		ASSERT_EQUALS(size, ByteBuffer::DEFAULT_INITIAL_SIZE);
	}

}