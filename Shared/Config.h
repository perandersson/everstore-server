#ifndef _EVERSTORE_CONFIG_H_
#define _EVERSTORE_CONFIG_H_

#include <string>
#include <cinttypes>
#include "Log/Log.hpp"
#include "File/Path.hpp"

using std::string;

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

// The default log level used by the server
#define DEFAULT_LOG_LEVEL Log::Info

struct Config
{
	const string rootDir;
	const Path configPath;
	const string journalDir;
	const uint32_t numWorkers;
	const uint32_t maxConnections;
	const uint16_t port;
	const uint32_t maxJournalLifeTime;
	const uint32_t maxBufferSize;
	const uint32_t logLevel;

	Config(const string& rootDir, const Path& configPath, const string& journalDir, const uint32_t numWorkers,
	       const uint32_t maxConnections,
	       const uint16_t port, const uint32_t maxJournalLifeTime, uint32_t maxBufferSize, uint32_t logLevel) :
			rootDir(rootDir), configPath(configPath), journalDir(journalDir), numWorkers(numWorkers),
			maxConnections(maxConnections), port(port), maxJournalLifeTime(maxJournalLifeTime),
			maxBufferSize(maxBufferSize), logLevel(logLevel) {}

	// Converts the supplied command string into a currently-working directory string
	static string getWorkingDirectory(char* command);

	// Read the application properties from the supplied config filename
	static Config readFromConfigFile(const string& rootDir, const Path& configPath);
};

#endif
