#include "../Shared/everstore.h"
#include "test/Test.h"

TEST_SUITE(Config)
{
	UNIT_TEST(defaultValuesForNonExistentPropertiesFile) {
		const Path rootDir(Path::GetWorkingDirectory());
		const Path configFilename("test-resources/filenotfound.properties");

		const Config p = Config::readFromConfigFile(rootDir, configFilename);

		assertEquals(rootDir, p.rootDir);
		assertEquals(configFilename, p.configPath);
		assertEquals(Path(string(DEFAULT_JOURNAL_DIR)), p.journalDir);
		assertEquals((uint32_t) DEFAULT_NUM_WORKERS, p.numWorkers);
		assertEquals((uint32_t) DEFAULT_MAX_CONNECTIONS, p.maxConnections);
		assertEquals((uint16_t) DEFAULT_PORT, p.port);
		assertEquals((uint32_t) DEFAULT_JOURNAL_GC_SECONDS, p.maxJournalLifeTime);
		assertEquals((uint32_t) DEFAULT_MAX_DATA_SEND_SIZE, p.maxBufferSize);
		assertEquals((uint32_t) DEFAULT_LOG_LEVEL, p.logLevel);
	}

	UNIT_TEST(overrideAllPropertiesFromFile) {
		const Config p = Config::readFromConfigFile(Path(Path::GetWorkingDirectory()),
		                                            Path("test-resources/override_all.properties"));

		assertEquals(Path(string("path/to/journal")), p.journalDir);
		assertEquals(1U, p.numWorkers);
		assertEquals(2U, p.maxConnections);
		assertEquals((uint16_t) 1234, p.port);
		assertEquals(123456U, p.maxJournalLifeTime);
	}

	UNIT_TEST(overrideOnePropertyFromFile) {
		const Config p = Config::readFromConfigFile(Path(Path::GetWorkingDirectory()),
			Path("test-resources/override_one.properties"));

		assertEquals(Path(string(DEFAULT_JOURNAL_DIR)), p.journalDir);
		assertEquals((uint32_t) DEFAULT_NUM_WORKERS, p.numWorkers);
		assertEquals((uint32_t) DEFAULT_MAX_CONNECTIONS, p.maxConnections);
		assertEquals((uint16_t) DEFAULT_PORT, p.port);
		assertEquals(123456u, p.maxJournalLifeTime);
		assertEquals((uint32_t) DEFAULT_MAX_DATA_SEND_SIZE, p.maxBufferSize);
		assertEquals((uint32_t) DEFAULT_LOG_LEVEL, p.logLevel);
	}
}