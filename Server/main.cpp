#include <iostream>
#include <execinfo.h>
#include "Store.h"
#include "../Shared/File/Path.hpp"

Store* gEventStore = nullptr;

void HandleSevereError(int sig) {
	// Get void*'s for all entries on the stack
	void *array[10];
	size_t size = backtrace(array, 10);

	// Print out all the frames to stderr
	Log::Write(Log::Error, "Fatal signal %d has been raised", sig);
	backtrace_symbols_fd(array, size, STDERR_FILENO);
	exit(1);
}

void HandleSignal(int sig) {
	switch(sig) {
		case SIGINT:
		case SIGTERM:
			Log::Write(Log::Info, "Stopping server");
			if (gEventStore != nullptr) {
				gEventStore->stop();
			}
			break;
		default:
			HandleSevereError(sig);
	}
}

Path GetConfigPath(const Path& rootPath, int argc, char** argv) {
	Path configFileName = rootPath + Path(string(DEFAULT_CONFIG_FILENAME));
	for (int i = 0; i < argc; ++i) {
		const auto configKey = argv[i];
		if (strcmp("--config", configKey) == 0) {
			i++;
			if (i < argc) {
				const auto configValue = argv[i];
				configFileName = Path(configValue);
			}
		}
	}
	return Path(configFileName);
}

void DisplayStartupConfig(const Config& config) {
	Log::Write(Log::Info, "Starting server");
	Log::Write(Log::Info, "Trying to load configuration from path: \"%s\"", config.configPath.value.c_str());
	Log::Write(Log::Info, "journalDir = \"%s\"", config.journalDir.value.c_str());
	Log::Write(Log::Info, "numWorker = %d", config.numWorkers);
	Log::Write(Log::Info, "maxConnections = %d", config.maxConnections);
	Log::Write(Log::Info, "port = %d", config.port);
	Log::Write(Log::Info, "maxBufferSize = %d", config.maxBufferSize);
	Log::Write(Log::Info, "logLevel = %d", config.logLevel);
}

int Start(const Config& config) {
	// Register so that we get events when a signal is being sent to us. This makes it possible for us to
	// gracefully shutdown the application. We are also interested in some of the fatal error signals.
	signal(SIGINT, HandleSignal);
	signal(SIGTERM, HandleSignal);
	signal(SIGSEGV, HandleSignal);
	signal(SIGSYS, HandleSignal);
	signal(SIGBUS, HandleSignal);
	signal(SIGILL, HandleSignal);
	signal(SIGPIPE, HandleSignal);
	signal(SIGABRT, HandleSignal);
	signal(SIGURG, HandleSignal);
	signal(SIGCHLD, HandleSignal);
#if defined(_WIN32)
	signal(SIGBREAK, handleSingal);
#endif

	gEventStore = new Store(config);
	const auto err = gEventStore->start();
	delete gEventStore;
	gEventStore = nullptr;

	// Log the error if one happens
	if (isError(err)) {
		Log::Write(Log::Error, "An unhandled error has occurred: %s (%d)", parseErrorCode(err), err);
		return 1;
	}

	// Whee!! Success!
	Log::Write(Log::Info, "Server is now stopped");
	return 0;
}

int main(int argc, char** argv) {
	const auto rootPath = Config::getWorkingDirectory(argv[0]);
	const auto configPath = GetConfigPath(rootPath, argc, argv);
	const auto config = Config::readFromConfigFile(rootPath, configPath);

	Log::SetLogLevel(config.logLevel);
	DisplayStartupConfig(config);

	// Start the server
	const auto ret = Start(config);
	std::flush(std::cout);
	std::flush(std::cerr);
	return ret;
}
