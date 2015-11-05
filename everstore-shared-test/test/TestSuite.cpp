#include "TestSuite.h"
#include "ITestCallback.h"
#include "UnitTest.h"
#include "TestException.h"

TestSuite::TestSuite(const std::string& name) : mName(name) {
}

TestSuite::~TestSuite() {
}

void TestSuite::runUnitTests(ITestCallback* callback) {
	callback->beginTestSuite(this);

	UnitTests::iterator it = mUnitTests.begin();
	UnitTests::iterator end = mUnitTests.end();
	for(;it != end; ++it) {
		UnitTest* test = *it;
		callback->beginUnitTest(this, test);
		try {
			test->Run(this);
		} catch(TestException e) {
			callback->testFailure(this, test, e.File.c_str(), e.Message.c_str(), e.Line);
		}
		callback->endUnitTest(this, test);
	}

	callback->endTestSuite(this);
}

void TestSuite::addUnitTest(UnitTest* test) {
	mUnitTests.push_back(test);
}
