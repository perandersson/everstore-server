#include "TestRunner.h"
#include "ITestCallback.h"
#include "TestSuite.h"
#include "UnitTest.h"
#include <iostream>
#include <sstream>

TestRunner::TestRunner()
{
}

TestRunner::~TestRunner()
{
}

TestRunner& TestRunner::get()
{
	static TestRunner obj;
	return obj;
}

namespace {
	class ConsoleOutputCallback : public ITestCallback
	{
	public:
		ConsoleOutputCallback() {}
		virtual ~ConsoleOutputCallback() {}

		virtual void begin() {
			std::cout << std::endl << "STARTS RUNNING TESTS" << std::endl;
			std::cout << std::endl;
		}

		virtual void end(){
		}

		virtual void beginTestSuite(const TestSuite* suite) {
			std::cout << "[" << suite->name() << "]" << std::endl;
		}
		
		virtual void endTestSuite(const TestSuite* suite) {
			std::cout << std::endl;
		}
		
		virtual void beginUnitTest(const TestSuite* suite, const UnitTest* test) {
			std::cout << "\tRunning: " << test->name();
		}

		virtual void endUnitTest(const TestSuite* suite, const UnitTest* test) {
			if(mFailures.empty()) {
				std::cout << " (OK)" << std::endl;
			} else {
				std::cout << " (FAILED) " << std::endl;
				for(size_t i = 0; i < mFailures.size(); ++i) {
					std::cout << mFailures[i] << std::endl;
				}
			}
			mFailures.clear();
		}

		virtual void testFailure(const TestSuite* suite, const UnitTest* test, const char* file, const char* message, long line) {
			std::stringstream ss;
			ss << "\t" << message << std::endl << "\t" << file << "(" << line << ")";
			mFailures.push_back(ss.str());
		}

	private:
		std::vector<std::string> mFailures;
	};

	class WrappedOutputCallback : public ITestCallback
	{
	public:
		WrappedOutputCallback(ITestCallback* callback)
			: mCallback(callback), mError(false) {}
		virtual ~WrappedOutputCallback() {}

		virtual void begin() {
			mCallback->begin();
		}

		virtual void end() {
			mCallback->end();
		}

		virtual void beginTestSuite(const TestSuite* suite) {
			mCallback->beginTestSuite(suite);
		}
		
		virtual void endTestSuite(const TestSuite* suite) {
			mCallback->endTestSuite(suite);
		}

		virtual void beginUnitTest(const TestSuite* suite, const UnitTest* test) {
			mCallback->beginUnitTest(suite, test);
		}

		virtual void endUnitTest(const TestSuite* suite, const UnitTest* test) {
			mCallback->endUnitTest(suite, test);
		}

		virtual void testFailure(const TestSuite* suite, const UnitTest* test, const char* file, const char* message, long line) {
			mCallback->testFailure(suite, test, file, message, line);
			mError = true;
		}

		inline bool IsError() const {
			return mError;
		}

	private:
		ITestCallback* mCallback;
		bool mError;
	};
}

int TestRunner::run()
{
	ConsoleOutputCallback callback;
	return run(&callback);
}

int TestRunner::run(ITestCallback* callback)
{
	WrappedOutputCallback wrapped(callback);
	wrapped.begin();

	TestSuites& suites = get().mSuites;
	TestSuites::iterator it = suites.begin();
	TestSuites::iterator end = suites.end();
	for(;it != end; ++it) {
		TestSuite* suite = it->second;
		suite->runUnitTests(&wrapped);
	}

	return wrapped.IsError() ? -1 : 0;
}

void TestRunner::addUnitTest(const char* suiteName, UnitTest* unitTest)
{
	TestSuites& suites = get().mSuites;

	std::string name(suiteName);
	TestSuites::iterator i = suites.find(name);
	TestSuite* suite = NULL;
	if(i != suites.end()) {
		suite = i->second;
	} else {
		suite = new TestSuite(suiteName);
		suites.insert(std::make_pair(name, suite));
	}
	suite->addUnitTest(unitTest);
}
