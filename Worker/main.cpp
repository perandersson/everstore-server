#include "../Shared/everstore.h"
#include "Worker.h"

Worker* gWorker = nullptr;

void handleSingal(int signal) {
	if (gWorker != nullptr) {
		gWorker->stop();
	}
}

int start(ChildProcessID idx, const Config& config) {
	gWorker = new Worker(idx, config);
	ESErrorCode err = gWorker->start();
	if (isError(err))
		gWorker->error(err);

	delete gWorker;
	return 0;
}

int main(int argc, char** argv) {
	// Register so that we get events when a signal is being sent to us. This makes it possible for us to
	// gracefully shutdown the application.
	signal(SIGINT, handleSingal);
	signal(SIGTERM, handleSingal);

	if (argc != 3) {
		printf("Expected: everstore-worker <idx> <configPath>\n");
		return 1;
	}

	// Read the neccessary configuration for the worker
	const auto rootPath = Config::getWorkingDirectory(argv[0]);
	const string configFileName(argv[2]);
	const auto p = Config::readFromConfigFile(rootPath, configFileName);

	// Start the worker
	const ChildProcessID id(atoi(argv[1]));
	return start(id, p);
}