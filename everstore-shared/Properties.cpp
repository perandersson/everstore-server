#include "Properties.h"
#include "FileUtils.h"
#include <fstream>
#include <algorithm>

using namespace std;

string Properties::getWorkingDirectory(char* command) {
	string result(command);
	auto idx = result.find_last_of('/');
	if (idx == string::npos)
		idx = result.find_last_of('\\');
	return result.substr(0, idx);
}

Properties Properties::readFromConfigFile(const string& rootDir, const string& configFileName) {
	string journalDir = DEFAULT_JOURNAL_DIR;
	uint32_t numWorkers = DEFAULT_NUM_WORKERS;
	uint32_t maxConnections = DEFAULT_MAX_CONNECTIONS;
	uint16_t port = DEFAULT_PORT;
	uint32_t maxDataSendSize = DEFAULT_MAX_DATA_SEND_SIZE;
	uint32_t maxJournalLifeTime = DEFAULT_JOURNAL_GC_SECONDS;

	ifstream file;
	file.open(configFileName);
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
					journalDir = value;
				} else if (key == string("numWorkers")) {
					numWorkers = StringUtils::toUint32(value);
				} else if (key == string("maxConnections")) {
					maxConnections = StringUtils::toUint32(value);
				} else if (key == string("port")) {
					port = StringUtils::toUint16(value);
				} else if (key == string("maxDataSendSize")) {
					maxDataSendSize = StringUtils::toUint32(value);
				} else if (key == string("maxJournalLifeTime")) {
					maxJournalLifeTime = StringUtils::toUint32(value);
				}
			}
		}
		file.close();
	}

	return Properties(rootDir, configFileName, journalDir, numWorkers, maxConnections, port, maxDataSendSize,
	                  maxJournalLifeTime);
}
