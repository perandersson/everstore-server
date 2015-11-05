#include <everstore.h>
#include "Store.h"

Store* gEventStore;

void handleSingal(int signal) {
	printf("Stopping server\n");
	gEventStore->stop();
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

void printServerProperties(const Properties& props) {
	printf("Trying to load configuration from path: \"%s\"\n", props.configFilename.c_str());
	printf("journalDir = \"%s\"\n", props.journalDir.c_str());
	printf("numWorker = %d\n", props.numWorkers);
	printf("maxConnections = %d\n", props.maxConnections);
	printf("port = %d\n", props.port);
	printf("maxJournalLifeTime = %d\n", props.maxJournalLifeTime);
}

int main(int argc, char** argv) {
	const string rootPath = Properties::getWorkingDirectory(argv[0]);
	const string configFileName = getConfigPath(rootPath, argc, argv);
	const Properties props = Properties::readFromConfigFile(rootPath, configFileName);

	gEventStore = new Store(props);
	printf("Starting up everstore-server\n");

	printServerProperties(props);

	signal(SIGINT, handleSingal);
	signal(SIGTERM, handleSingal);
	const ESErrorCode err = gEventStore->start();
	if (isError(err)) {
		printf("Could not start host: %s", parseErrorCode(err));
	}

	delete gEventStore;
	return 0;
}
