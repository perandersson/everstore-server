#pragma once
#include <exception>
#include <string>

class UnitTest;

//
// Base class for test failures inside the test framework
class TestException : public std::exception
{
public:
	TestException(UnitTest* test, const char* file, const char* message, long line);
	~TestException();

public:
	const UnitTest* Test;
	const std::string File;
	const std::string Message;
	const long Line;
};
