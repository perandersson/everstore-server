#pragma once
#include <string>
#include <map>

class TestSuite;
class UnitTest;
class ITestCallback;

//
// 
class TestRunner
{
	typedef std::map<std::string, TestSuite*> TestSuites;

public:
	~TestRunner();

protected:
	TestRunner();

	// 
	static TestRunner& get();

public:
	//
	// @return The return code 
	static int run();

	//
	// @return The return code 
	// @param callback
	static int run(ITestCallback* callback);

	//
	// Adds a unit test in the supplied suite
	// @param suiteName The name of the test suite
	// @param unitTest
	static void addUnitTest(const char* suiteName, UnitTest* unitTest);

private:
	TestSuites mSuites;
};
