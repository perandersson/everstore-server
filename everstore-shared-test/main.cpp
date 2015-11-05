#include <iostream>
#include <stdlib.h>
#include <ctime>
#include "test/Test.h"
#include <everstore.h>

int main(int argc, char** argv) {
	const string tempDir = FileUtils::getTempDirectory();
	FileUtils::clearAndDeleteDirectory(tempDir);
	srand(time(NULL));

	return TestRunner::run();
}
