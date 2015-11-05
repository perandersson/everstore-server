#include "Test.h"

std::string toString(int value) {
	char tmp[1024];
	sprintf(tmp, "%d", value);
	return std::string(tmp);
}

std::string toString(uint16_t value) {
	char tmp[1024];
	sprintf(tmp, "%d", value);
	return std::string(tmp);
}

std::string toString(uint32_t value) {
	char tmp[1024];
	sprintf(tmp, "%u", value);
	return std::string(tmp);
}

std::string toString(std::string value) {
	return value;
}

std::string toString(unsigned char value) {
	char tmp[1024];
	sprintf(tmp, "%c", value);
	return tmp;
}

void innerAssertNull(void* value, UnitTest* test, const char* file, long line) {
	if (value != nullptr) {
		throw TestException(test, file, "expected: <nullptr> but was: non-nullptr", line);
	}
}

void innerAssertNotNull(void* value, UnitTest* test, const char* file, long line) {
	if (value == nullptr) {
		throw TestException(test, file, "expected: non-nullptr but was: <nullptr>", line);
	}
}

void innerAssertTrue(bool value, UnitTest* test, const char* file, long line) {
	if (!value) {
		throw TestException(test, file, "expected: <true> but was: <false>", line);
	}
}

void innerAssertFalse(bool value, UnitTest* test, const char* file, long line) {
	if (value) {
		throw TestException(test, file, "expected: <false> but was: <true>", line);
	}
}
