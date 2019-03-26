#pragma once
#include <string>
#include <list>
#include <vector>

class UnitTest;
class ITestCallback;

class TestSuite
{
	typedef std::list<UnitTest*> UnitTests;

public:
	TestSuite(const std::string& name);
	~TestSuite();

	//
	// Run all tests located in this test-suite
	void runUnitTests(ITestCallback* callback);

	//
	// Add a unit test in this test-suite
	// @param test
	void addUnitTest(UnitTest* test);

	// Retrieves the name for this suite
	inline const std::string& name() const { return mName; }

private:
	UnitTests mUnitTests;
	std::string mName;
};
