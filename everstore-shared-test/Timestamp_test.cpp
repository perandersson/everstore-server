#include <everstore.h>
#include "test/Test.h"

#include <regex>
using namespace std;

TEST_SUITE(Timestamp) {
	UNIT_TEST(validTimestampFormat) {
		Timestamp t;

		assertTrue(regex_match(string(t.value), regex("\\d{4}-\\d{2}-\\d{2}T\\d{2}:\\d{2}:\\d{2}\\.\\d{3}")));
	}
}
