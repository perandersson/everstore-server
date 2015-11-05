#include "UnitTest.h"
#include "TestSuite.h"

UnitTest::UnitTest(const char* name, const char* file, const long line)
	: mName(name), mFile(file), mLine(line)
{
}

UnitTest::~UnitTest()
{
}
