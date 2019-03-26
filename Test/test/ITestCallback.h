#pragma once

class TestSuite;
class UnitTest;

//
// 
class ITestCallback
{
public:
	virtual ~ITestCallback() {}

	//
	// Method invoked when the test application begins it's execution
	virtual void begin() = 0;

	//
	// Method invoked when the test application ends it's execution
	virtual void end() = 0;

	//
	// Method invoked when a test suite begins it's execution
	// @param suite
	virtual void beginTestSuite(const TestSuite* suite) = 0;
		
	//
	// Method invoked when a test suite has completed it's execution
	// @param suite
	virtual void endTestSuite(const TestSuite* suite) = 0;

	//
	// Method invoked when a unit test begins it's execution
	// @param suite
	// @param test
	virtual void beginUnitTest(const TestSuite* suite, const UnitTest* test) = 0;

	//
	// Method invoked when a unit test has completed it's execution
	// @param suite
	// @param test
	virtual void endUnitTest(const TestSuite* suite, const UnitTest* test) = 0;

	//
	// Method invoked when a unit test fails.
	virtual void testFailure(const TestSuite* suite, const UnitTest* test, const char* file, const char* message, long line) = 0;
};
