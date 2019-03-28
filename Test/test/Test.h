#pragma once
#include "TestSuite.h"
#include "ITestCallback.h"
#include "TestRunner.h"
#include "UnitTest.h"
#include "TestException.h"
#include <cstdint>
#include "../../Shared/File/Path.hpp"

#define TEST_SUITE(Name) \
	namespace Name##_suite { static const char SuiteName[] = {#Name}; } \
	namespace Name##_suite
	
#define UNIT_TEST(Name) \
	void Name##_runner(TestSuite* suite, UnitTest* unitTest); \
	class Name##_UnitTest : public UnitTest { \
	public: \
		Name##_UnitTest() : UnitTest(#Name, __FILE__, __LINE__) { \
			TestRunner::addUnitTest(SuiteName, this); \
		} \
		virtual void Run(TestSuite* suite) { Name##_runner(suite, this); } \
	} _##Name##_UnitTest_instance; \
	void Name##_runner(TestSuite* suite, UnitTest* unitTest)

std::string toString(int value);
std::string toString(uint16_t value);
std::string toString(uint32_t value);
std::string toString(unsigned char value);
std::string toString(std::string value);
std::string toString(const Path& value);

void innerAssertNull(void* ptr, UnitTest* test, const char* file, long line);
void innerAssertNotNull(void* ptr, UnitTest* test, const char* file, long line);
void innerAssertTrue(bool value, UnitTest* test, const char* file, long line);
void innerAssertFalse(bool value, UnitTest* test, const char* file, long line);

template<typename T>
void innerAssertEquals(const T expected, const T value, 
	UnitTest* test, const char* file, long line) {
	if (value != expected) {
		const std::string expectedStr = toString(expected);
		const std::string valueStr = toString(value);
		
		char tmp[1024];
		sprintf(tmp, "expected: <%s> but was: <%s>", expectedStr.c_str(), valueStr.c_str());

		throw TestException(test, file, tmp, line);
	}
}

#define assertTrue(value) innerAssertTrue((value), unitTest, __FILE__, __LINE__)
#define assertFalse(value) innerAssertFalse((value), unitTest, __FILE__, __LINE__)
#define assertEquals(expected, value) innerAssertEquals((expected), (value), unitTest, __FILE__, __LINE__)
#define assertNull(value) innerAssertNull((value), unitTest, __FILE__, __LINE__)
#define assertNotNull(value) innerAssertNotNull((value), unitTest, __FILE__, __LINE__)
#define assertFail(message) throw TestException(unitTest, __FILE__, message, __LINE__);

// Global test-suite name.
// @remark Use the TEST_SUITE macro for grouping unit tests together.
static const char SuiteName[] = {"Global Suite"};
