#ifndef _EVERSTORE_PROPERTIES_H_
#define _EVERSTORE_PROPERTIES_H_

#include "es_config.h"

struct Properties {
	const string rootDir;
	const string configFilename;
	const string journalDir;
	const uint32_t numWorkers;
	const uint32_t maxConnections;
	const uint16_t port;
	const uint32_t maxJournalLifeTime;
	
	Properties(const string& rootDir, const string& configFilename, const string& journalDir, const uint32_t numWorkers, const uint32_t maxConnections,
		const uint16_t port, const uint32_t maxJournalLifeTime) : 
		rootDir(rootDir), configFilename(configFilename), journalDir(journalDir), numWorkers(numWorkers),
		maxConnections(maxConnections), port(port), maxJournalLifeTime(maxJournalLifeTime) {}

	// Converts the supplied command string into a currently-working directory string
	static string getWorkingDirectory(char* command);

	// Read the application properties from the supplied config filename
	static Properties readFromConfigFile(const string& rootDir, const string& configFileName);
};

#endif
