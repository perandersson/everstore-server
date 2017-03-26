#ifndef _EVERSTORE_PROPERTIES_H_
#define _EVERSTORE_PROPERTIES_H_

#include <string>
#include <cinttypes>

using std::string;

struct Properties
{
	const string rootDir;
	const string configFilename;
	const string journalDir;
	const uint32_t numWorkers;
	const uint32_t maxConnections;
	const uint16_t port;
	const uint32_t maxDataSendSize;
	const uint32_t maxJournalLifeTime;

	Properties(const string& rootDir, const string& configFilename, const string& journalDir, const uint32_t numWorkers,
	           const uint32_t maxConnections,
	           const uint16_t port, const uint32_t maxDataSendSize,
	           const uint32_t maxJournalLifeTime) :
			rootDir(rootDir), configFilename(configFilename), journalDir(journalDir), numWorkers(numWorkers),
			maxConnections(maxConnections), port(port), maxDataSendSize(maxDataSendSize),
			maxJournalLifeTime(maxJournalLifeTime) {}

	// Converts the supplied command string into a currently-working directory string
	static string getWorkingDirectory(char* command);

	// Read the application properties from the supplied config filename
	static Properties readFromConfigFile(const string& rootDir, const string& configFileName);
};

// How many child processes is started up by default
#define DEFAULT_NUM_WORKERS 5

// The default connection port for clients
#define DEFAULT_PORT 6929

// How many simulatnious connections are allowed at the same time
#define DEFAULT_MAX_CONNECTIONS 10

// Default data directory
#define DEFAULT_JOURNAL_DIR "journals"

// Default config filename
#define DEFAULT_CONFIG_FILENAME "settings.conf"

// How large is the maximum data-block size sent over the network (64kb)
#define DEFAULT_MAX_DATA_SEND_SIZE 65536

// The default socket timeout for reading and writing
//#define DEFAULT_SOCKET_TIMEOUT 2000

// How many seconds we wait until un-accessed journals are deleted
#define DEFAULT_JOURNAL_GC_SECONDS 60

#endif
