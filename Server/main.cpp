#include "Store.h"

Store* gEventStore = nullptr;

void handleSingal(int signal) {
	printf("Stopping server\n");
	if (gEventStore != nullptr) {
		gEventStore->stop();
	}
	printf("Server stopped\n");
}

string getConfigPath(const string& rootPath, int argc, char** argv) {
	string configFileName = string(rootPath + string(FileUtils::PATH_DELIM) + DEFAULT_CONFIG_FILENAME);
	for (int i = 0; i < argc; ++i) {
		const auto configKey = argv[i];
		if (strcmp("--config", configKey) == 0) {
			i++;
			if (i < argc) {
				const auto configValue = argv[i];
				configFileName = configValue;
			}
		}
	}
	return configFileName;
}

void printServerProperties(const Config& config) {
	Log::Write(Log::Info, "Trying to load configuration from path: \"%s\"", config.configFilename.c_str());
	Log::Write(Log::Info, "journalDir = \"%s\"", config.journalDir.c_str());
	Log::Write(Log::Info, "numWorker = %d", config.numWorkers);
	Log::Write(Log::Info, "maxConnections = %d", config.maxConnections);
	Log::Write(Log::Info, "port = %d", config.port);
	Log::Write(Log::Info, "maxBufferSize = %d", config.maxBufferSize);
	Log::Write(Log::Info, "logLevel = %d", config.logLevel);
}

int main(int argc, char** argv) {
	const string rootPath = Config::getWorkingDirectory(argv[0]);
	const string configFileName = getConfigPath(rootPath, argc, argv);
	const Config props = Config::readFromConfigFile(rootPath, configFileName);
	Log::SetLogLevel(props.logLevel);

	gEventStore = new Store(props);
	printf("Starting up Server\n");

	printServerProperties(props);

	// Register so that we get events when a signal is being sent to us. This makes it possible for us to
	// gracefully shutdown the application.
	signal(SIGINT, handleSingal);
	signal(SIGTERM, handleSingal);

	// Start the server
	const auto err = gEventStore->start();
	delete gEventStore;
	gEventStore = nullptr;

	if (isError(err)) {
		Log::Write(Log::Error, "An unhandled error has occurred: %s (%d)", parseErrorCode(err), err);
	}
	return 0;
}
