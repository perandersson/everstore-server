#include <cstdlib>
#include <ctime>
#include <iostream>
#include "test/Test.h"
#include "../Shared/everstore.h"

int main(int argc, char** argv) {
	const auto tempDir = FileUtils::getTempDirectory();
	const auto currentDir = Path::GetWorkingDirectory();
	std::cout << "Temp dir: " << tempDir << std::endl;
	std::cout << "Current dir: " << currentDir.value << std::endl;

	FileUtils::clearAndDeleteDirectory(tempDir);
	srand(time(nullptr));
	return TestRunner::run();
}
