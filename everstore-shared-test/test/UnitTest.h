#pragma once
#include <string>

class TestSuite;

//
// Base class for the unit tests run in the test framework
class UnitTest
{
public:
	UnitTest(const char* name, const char* file, const long line);
	virtual ~UnitTest();

	//
	// Run this test inside the supplied test suite
	// @param suite The test suite
	virtual void Run(TestSuite* suite) = 0;

	// Retrieves the name of this test-case 
	inline const std::string& name() const { return mName; }

	// Retrieves the filename where the test-case is located
	inline const std::string& fileName() const { return mFile; }

	// Retrieves the line where the test-case is located
	inline const long line() const { return mLine; }

private:
	const std::string mName;
	const std::string mFile;
	const long mLine;
};
