#include <everstore.h>
#include "test/Test.h"

TEST_SUITE(Bytes) {

	UNIT_TEST(bytesCapacity) {
		const auto BYTES_INIT_SIZE = 1234U;
		Bytes bytes(BYTES_INIT_SIZE);

		assertEquals(BYTES_INIT_SIZE, bytes.capacity());
	}

	UNIT_TEST(bytesEnsureCapacityNoResize) {
		const auto BYTES_INIT_SIZE = 10U;
		Bytes bytes(BYTES_INIT_SIZE);

		assertEquals(BYTES_INIT_SIZE, bytes.capacity());

		bytes.ensureCapacity(5U);

		assertEquals(BYTES_INIT_SIZE, bytes.capacity());
	}

	UNIT_TEST(bytesEnsureCapacityResize) {
		const auto BYTES_INIT_SIZE = 10U;
		Bytes bytes(BYTES_INIT_SIZE);

		assertEquals(BYTES_INIT_SIZE, bytes.capacity());

		bytes.ensureCapacity(15U);

		assertEquals(15U, bytes.capacity());
	}

	UNIT_TEST(bytesMoveForwardNonClamped) {
		const auto BYTES_INIT_SIZE = 10U;
		Bytes bytes(BYTES_INIT_SIZE);

		bytes.moveForward(2U);

		assertEquals(2U, bytes.offset());
	}

	UNIT_TEST(bytesMoveForwardClamped) {
		const auto BYTES_INIT_SIZE = 10U;
		Bytes bytes(BYTES_INIT_SIZE);

		bytes.moveForward(12U);

		assertEquals(BYTES_INIT_SIZE, bytes.offset());
	}

	UNIT_TEST(bytesMoveBackwardsNoClamped) {
		const auto BYTES_INIT_SIZE = 10U;
		Bytes bytes(BYTES_INIT_SIZE);

		bytes.moveForward(5U);
		bytes.moveBackwards(2U);

		assertEquals(3U, bytes.offset());
	}

	UNIT_TEST(bytesMoveBackwardsClamped) {
		const auto BYTES_INIT_SIZE = 10U;
		Bytes bytes(BYTES_INIT_SIZE);

		bytes.moveForward(5U);
		bytes.moveBackwards(6U);

		assertEquals(0U, bytes.offset());
	}
}
