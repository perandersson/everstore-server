#include <everstore.h>
#include "test/Test.h"

TEST_SUITE(Properties) {
	UNIT_TEST(defaultValuesForNonExistentPropertiesFile) {
		const string rootDir(".");
		const string configFilename("test-resources/filenotfound.properties");

		const Properties p = Properties::readFromConfigFile(rootDir, configFilename);

		assertEquals(rootDir, p.rootDir);
		assertEquals(configFilename, p.configFilename);
		assertEquals(string(DEFAULT_JOURNAL_DIR), p.journalDir);
		assertEquals((uint32_t)DEFAULT_NUM_WORKERS, p.numWorkers);
		assertEquals((uint32_t)DEFAULT_MAX_CONNECTIONS, p.maxConnections);
		assertEquals((uint16_t)DEFAULT_PORT, p.port);
		assertEquals((uint32_t)DEFAULT_JOURNAL_GC_SECONDS, p.maxJournalLifeTime);
	}

	UNIT_TEST(overrideAllPropertiesFromFile) {
		const Properties p = Properties::readFromConfigFile(string("."), 
			string("test-resources/override_all.properties"));

		assertEquals(string("path/to/journal"), p.journalDir);
		assertEquals(1U, p.numWorkers);
		assertEquals(2U, p.maxConnections);
		assertEquals((uint16_t)1234, p.port);
		assertEquals(123456U, p.maxJournalLifeTime);
	}

	UNIT_TEST(overrideOnePropertyFromFile) {
		const Properties p = Properties::readFromConfigFile(string("."),
			string("test-resources/override_one.properties"));

		assertEquals(string(DEFAULT_JOURNAL_DIR), p.journalDir);
		assertEquals((uint32_t)DEFAULT_NUM_WORKERS, p.numWorkers);
		assertEquals((uint32_t)DEFAULT_MAX_CONNECTIONS, p.maxConnections);
		assertEquals((uint16_t)DEFAULT_PORT, p.port);
		assertEquals(123456U, p.maxJournalLifeTime);
	}
}