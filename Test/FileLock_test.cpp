#include "../Shared/everstore.h"
#include "test/Test.h"

TEST_SUITE(FileLock) {
	UNIT_TEST(falseIfNotExistsStatic) {
		assertFalse(FileLock::exists("does/not/exists.lock"));
	}

	UNIT_TEST(falseIfNotExists) {
		const string tempPath = FileUtils::getTempFile();

		FileLock lock(tempPath);
		assertFalse(lock.exists());
	}

	UNIT_TEST(trueIfNotExists) {
		const string tempPath = FileUtils::getTempFile();

		FileLock lock(tempPath);
		lock.addRef();
		assertTrue(lock.exists());
		lock.release();
		assertFalse(lock.exists());
	}

	UNIT_TEST(trueIfNotExists2) {
		const string tempPath = FileUtils::getTempFile();

		FileLock lock(tempPath);
		lock.addRef();
		lock.addRef();
		assertTrue(lock.exists());
		lock.release();
		assertTrue(lock.exists());
		lock.release();
		assertFalse(lock.exists());
	}
}