#include <cstdlib>
#include <ctime>
#include "test/Test.h"
#include "../Shared/everstore.h"

int main(int argc, char** argv) {
	const string tempDir = FileUtils::getTempDirectory();
	FileUtils::clearAndDeleteDirectory(tempDir);
	srand(time(nullptr));

	return TestRunner::run();
}
