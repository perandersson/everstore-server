#include "Store.h"
#include "../Shared/File/Path.hpp"

Store* gEventStore = nullptr;

void handleSingal(int) {
	Log::Write(Log::Info, "Stopping server");
	if (gEventStore != nullptr) {
		gEventStore->stop();
	}
	Log::Write(Log::Info, "Server stopped");
}

Path getConfigPath(const Path& rootPath, int argc, char** argv) {
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

void printServerProperties(const Config& config) {
	Log::Write(Log::Info, "Trying to load configuration from path: \"%s\"", config.configPath.value.c_str());
	Log::Write(Log::Info, "journalDir = \"%s\"", config.journalDir.value.c_str());
	Log::Write(Log::Info, "numWorker = %d", config.numWorkers);
	Log::Write(Log::Info, "maxConnections = %d", config.maxConnections);
	Log::Write(Log::Info, "port = %d", config.port);
	Log::Write(Log::Info, "maxBufferSize = %d", config.maxBufferSize);
	Log::Write(Log::Info, "logLevel = %d", config.logLevel);
}

int main(int argc, char** argv) {
	const auto rootPath = Config::getWorkingDirectory(argv[0]);
	const auto configPath = getConfigPath(rootPath, argc, argv);
	const auto props = Config::readFromConfigFile(rootPath, configPath);

	Log::SetLogLevel(props.logLevel);
	Log::Write(Log::Info, "Starting server");
	printServerProperties(props);

	gEventStore = new Store(props);

	// Register so that we get events when a signal is being sent to us. This makes it possible for us to
	// gracefully shutdown the application.
	signal(SIGINT, handleSingal);
	signal(SIGTERM, handleSingal);
#if defined(_WIN32)
	signal(SIGBREAK, handleSingal);
#endif

	// Start the server
	const auto err = gEventStore->start();
	delete gEventStore;
	gEventStore = nullptr;

	if (isError(err)) {
		Log::Write(Log::Error, "An unhandled error has occurred: %s (%d)", parseErrorCode(err), err);
	}
	return 0;
}
