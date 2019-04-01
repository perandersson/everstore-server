#include "../Shared/everstore.h"
#include "Worker.h"
#include "../Shared/Log/Log.hpp"

Worker* gWorker = nullptr;

void handleSingal(int) {
	Log::Write(Log::Info, "Stopping worker");
	if (gWorker != nullptr) {
		gWorker->stop();
	}
	Log::Write(Log::Info, "Worker is not stopped");
}

void printWorkerProperties(const Config& config) {
	Log::Write(Log::Info, "Trying to load configuration from path: \"%s\"", config.configPath.value.c_str());
	Log::Write(Log::Info, "journalDir = \"%s\"", config.journalDir.c_str());
	Log::Write(Log::Info, "numWorker = %d", config.numWorkers);
	Log::Write(Log::Info, "maxConnections = %d", config.maxConnections);
	Log::Write(Log::Info, "port = %d", config.port);
	Log::Write(Log::Info, "maxBufferSize = %d", config.maxBufferSize);
	Log::Write(Log::Info, "logLevel = %d", config.logLevel);
}

int start(ProcessID idx, const Config& config) {
	// Register so that we get events when a signal is being sent to us. This makes it possible for us to
	// gracefully shutdown the application.
	signal(SIGINT, handleSingal);
	signal(SIGTERM, handleSingal);

	// Create and start the worker
	gWorker = new Worker(idx, config);
	const auto err = gWorker->start();
	delete gWorker;

	// Log the error if one happens
	if (isError(err)) {
		Log::Write(Log::Error, "An unhandled error has occurred: %s (%d)", parseErrorCode(err), err);
		return 1;
	}

	// Whee!! Success!
	Log::Write(Log::Info, "Shutting down Worker");
	return 0;
}

int main(int argc, char** argv) {
	if (argc != 3) {
		printf("Expected: everstore-worker <idx> <configPath>\n");
		return 1;
	}

	// Read the necessary configuration for the worker
	const auto rootPath = Config::getWorkingDirectory(argv[0]);
	const string configFileName(argv[2]);
	const auto p = Config::readFromConfigFile(rootPath, Path(configFileName));
	Log::SetLogLevel(p.logLevel);
	printWorkerProperties(p);

	// Start the worker
	const ProcessID id(atoi(argv[1]));
	Log::SetChildProcessID(id);
	return start(id, p);
}