#include "Config.h"
#include "File/FileUtils.h"
#include <fstream>
#include <algorithm>

using namespace std;

Path Config::getWorkingDirectory(char* command) {
	string result(command);
	auto idx = result.find_last_of('/');
	if (idx == -1)
		idx = result.find_last_of('\\');
	return Path(result.substr(0, idx));
}

Config Config::readFromConfigFile(const Path& rootDir, const Path& configPath) {
	Path journalDir(string(DEFAULT_JOURNAL_DIR));
	uint32_t numWorkers = DEFAULT_NUM_WORKERS;
	uint32_t maxConnections = DEFAULT_MAX_CONNECTIONS;
	uint16_t port = DEFAULT_PORT;
	uint32_t maxJournalLifeTime = DEFAULT_JOURNAL_GC_SECONDS;
	uint32_t maxBufferSize = DEFAULT_MAX_DATA_SEND_SIZE;
	uint32_t logLevel = DEFAULT_LOG_LEVEL;

	ifstream file = configPath.OpenStream();
	if (file.is_open()) {
		while (!file.eof()) {
			string line;
			std::getline(file, line);

			vector<string> keyValuePair;
			StringUtils::split(line, '=', keyValuePair);
			if (keyValuePair.size() == 2) {
				const auto key = keyValuePair[0];
				const auto value = keyValuePair[1];

				if (key == string("journalDir")) {
					journalDir = Path(value);
				} else if (key == string("numWorkers")) {
					numWorkers = StringUtils::toUint32(value);
				} else if (key == string("maxConnections")) {
					maxConnections = StringUtils::toUint32(value);
				} else if (key == string("port")) {
					port = StringUtils::toUint16(value);
				} else if (key == string("maxJournalLifeTime")) {
					maxJournalLifeTime = StringUtils::toUint32(value);
				} else if (key == string("maxBufferSize")) {
					maxBufferSize = StringUtils::toUint32(value);
				} else if (key == string("logLevel")) {
					logLevel = StringUtils::toUint32(value);
				}
			}
		}
		file.close();
	}

	return Config(rootDir, configPath, journalDir, numWorkers, maxConnections, port, maxJournalLifeTime,
	              maxBufferSize, logLevel);
}
