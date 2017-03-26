#include "FileLock.h"
#include "FileUtils.h"
#include <cassert>

using std::lock_guard;

FileLock::FileLock(const string& path) : mPath(path), mCount(0) {
}

FileLock::~FileLock() {
	assert(mCount == 0 && "The amount of addRef and release does not match");
}

void FileLock::addRef() {
	lock_guard<mutex> l(mMutex);
	if (++mCount == 1) {
		// Create a lock file on the HDD
		FILE* fileHandle = fopen(mPath.c_str(), "a");
		fclose(fileHandle);
	}
}

void FileLock::release() {
	lock_guard<mutex> l(mMutex);
	if (--mCount == 0) {
		remove(mPath.c_str());
	}
}

bool FileLock::exists(const string& path) {
	return FileUtils::fileExists(path);
}

bool FileLock::exists() const {
	return FileUtils::fileExists(mPath);
}
