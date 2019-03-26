#ifndef _EVERSTORE_CONFIG_H_
#define _EVERSTORE_CONFIG_H_

#include "es_config.h"

struct Config {
	const string rootDir;
	const string configFilename;
	const string journalDir;
	const uint32_t numWorkers;
	const uint32_t maxConnections;
	const uint16_t port;
	const uint32_t maxJournalLifeTime;
	
	Config(const string& rootDir, const string& configFilename, const string& journalDir, const uint32_t numWorkers, const uint32_t maxConnections,
		const uint16_t port, const uint32_t maxJournalLifeTime) : 
		rootDir(rootDir), configFilename(configFilename), journalDir(journalDir), numWorkers(numWorkers),
		maxConnections(maxConnections), port(port), maxJournalLifeTime(maxJournalLifeTime) {}

	// Converts the supplied command string into a currently-working directory string
	static string getWorkingDirectory(char* command);

	// Read the application properties from the supplied config filename
	static Config readFromConfigFile(const string& rootDir, const string& configFileName);
};

#endif
