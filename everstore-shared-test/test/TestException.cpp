#include "TestException.h"
#include "UnitTest.h"

TestException::TestException(UnitTest* test, const char* file, const char* message, long line)
	: Test(test), File(file), Message(message), Line(line)
{}

TestException::~TestException()
{}
