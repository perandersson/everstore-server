#include "../Shared/everstore.h"
#include "Worker.h"

Worker* gWorker;

void handleSingal(int signal) {
	gWorker->stop();
}

int start(ChildProcessId idx, const Config& config) {
	gWorker = new Worker(idx, config);
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
	const string rootPath = Config::getWorkingDirectory(argv[0]);
	const string configFileName(argv[2]);
	const Config p = Config::readFromConfigFile(rootPath, configFileName);

	// Start the worker
	return start(atoi(argv[1]), p);
}