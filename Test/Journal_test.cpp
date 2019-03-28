#include "../Shared/everstore.h"
#include "test/Test.h"
#include <experimental/filesystem>

TEST_SUITE(Journal)
{
	static const string logSuffix(".log");

	UNIT_TEST(emptyJournalForNonExistingFile) {
		const Path tempPath(FileUtils::getTempFile() + logSuffix);
		Journal j(tempPath, ChildProcessID(1));

		assertEquals(0u, j.journalSize());
		assertEquals(tempPath, j.path());
	}

	UNIT_TEST(persistenceCheckOkEmptyJournal) {
		const Path tempPath(FileUtils::getTempFile() + logSuffix);
		Journal j(tempPath, ChildProcessID(1));

		assertTrue(j.performConsistencyCheck());
	}

	void manuallyReadJournal(UnitTest* unitTest, const string& path, char* dest, uint32_t* size) {
		FILE* file = fopen((path).c_str(), "r+b");
		assertNotNull(file);

		*size = FileUtils::getFileSize(file);

		fread(dest, *size, 1, file);
		fclose(file);
	}

	void manuallyCreateLockFile(const string& path) {
		string lockPath = path + string(".lock");
		FILE* fileHandle = fopen(lockPath.c_str(), "a");
		fclose(fileHandle);
	}

	UNIT_TEST(persistenceCheckOkForValidJournal) {
		const Path targetJournal(FileUtils::getTempFile() + logSuffix);
		const Path originalJournal("test-resources/journals/persistenceCheckOkForValidJournal.log");
		assertTrue(FileUtils::copyFile(originalJournal, targetJournal));

		manuallyCreateLockFile(targetJournal.value);

		Journal j(targetJournal);
		assertTrue(j.performConsistencyCheck());

		char temp[1024] = {0};
		uint32_t tempSize = 0;
		const string expectedJournal("test-resources/journals/persistenceCheckOkForValidJournal_expected.log");
		manuallyReadJournal(unitTest, expectedJournal, temp, &tempSize);

		ByteBuffer bb(1024);
		AutoClosable<FileInputStream>(j.inputStream(0))->readBytes(&bb);
		const char* ptr = bb.ptr();

		assertEquals(tempSize, bb.offset());
		for (uint32_t i = 0; i < tempSize; ++i) {
			char c1 = ptr[i];
			char c2 = temp[i];

			assertEquals(c1, c2);
		}
	}

	UNIT_TEST(persistenceCheckOkForJournal1) {
		const Path targetJournal(FileUtils::getTempFile() + logSuffix);
		const Path originalJournal("test-resources/journals/persistenceCheckOkForJournal1.log");
		assertTrue(FileUtils::copyFile(originalJournal, targetJournal));

		manuallyCreateLockFile(targetJournal.value);

		Journal j(targetJournal);
		assertTrue(j.performConsistencyCheck());

		char temp[1024] = {0};
		uint32_t tempSize = 0;
		const string expectedJournal("test-resources/journals/persistenceCheckOkForJournal1_expected.log");
		manuallyReadJournal(unitTest, expectedJournal, temp, &tempSize);

		ByteBuffer bb(1024);
		AutoClosable<FileInputStream>(j.inputStream(0))->readBytes(&bb);
		const char* ptr = bb.ptr();

		assertEquals(tempSize, bb.offset());
		for (uint32_t i = 0; i < tempSize; ++i) {
			char c1 = ptr[i];
			char c2 = temp[i];

			assertEquals(c1, c2);
		}
	}

	UNIT_TEST(persistenceCheckOkForJournal2) {
		const Path targetJournal(FileUtils::getTempFile() + logSuffix);
		const Path originalJournal("test-resources/journals/persistenceCheckOkForJournal2.log");
		assertTrue(FileUtils::copyFile(originalJournal, targetJournal));

		manuallyCreateLockFile(targetJournal.value);

		Journal j(targetJournal);
		assertTrue(j.performConsistencyCheck());

		char temp[1024] = {0};
		uint32_t tempSize = 0;
		const string expectedJournal("test-resources/journals/persistenceCheckOkForJournal2_expected.log");
		manuallyReadJournal(unitTest, expectedJournal, temp, &tempSize);

		ByteBuffer bb(1024);
		AutoClosable<FileInputStream>(j.inputStream(0))->readBytes(&bb);
		const char* ptr = bb.ptr();

		assertEquals(tempSize, bb.offset());
		for (uint32_t i = 0; i < tempSize; ++i) {
			char c1 = ptr[i];
			char c2 = temp[i];

			assertEquals(c1, c2);
		}
	}

	UNIT_TEST(persistenceCheckOkForJournal3) {
		const Path targetJournal(FileUtils::getTempFile() + logSuffix);
		const Path originalJournal("test-resources/journals/persistenceCheckOkForJournal3.log");
		assertTrue(FileUtils::copyFile(originalJournal, targetJournal));

		manuallyCreateLockFile(targetJournal.value);

		Journal j(targetJournal);
		assertTrue(j.performConsistencyCheck());
		assertEquals(0U, j.journalSize());

		auto const size = FileUtils::getFileSize(targetJournal.value);
		assertEquals(0u, size);
	}
}
