#include <everstore.h>
#include "Worker.h"

Worker* gWorker;

void handleSingal(int signal) {
	gWorker->stop();
}

int start(ChildProcessId idx, const Properties& props) {
	gWorker = new Worker(idx, props);
	ESErrorCode err = gWorker->start();
	if (isError(err))
		gWorker->error(err);
	
	delete gWorker;
	return 0;
}

int main(int argc, char** argv) {
	signal(SIGINT, handleSingal);
	signal(SIGTERM, handleSingal);

	if (argc != 3) {
		printf("Expected: everstore-worker <idx> <configPath>\n");
		return 1;
	}

	// Read the neccessary configuration for the worker
	const string rootPath = Properties::getWorkingDirectory(argv[0]);
	const string configFileName(argv[2]);
	const Properties p = Properties::readFromConfigFile(rootPath, configFileName);

	// Start the worker
	return start(atoi(argv[1]), p);
}