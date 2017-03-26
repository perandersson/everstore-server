#pragma once

#include <string>
#include <cinttypes>
#include <atomic>
#include <mutex>

using std::string;
using std::atomic;
using std::mutex;

/**
 * Reference counted file lock. Useful for knowing when a journal is in use or not.
 */
class FileLock
{
public:
	FileLock(const string& path);

	~FileLock();

	// Increase the reference counter for the file lock
	void addRef();

	// Decrease the reference counter for the file lock
	void release();

	// Check to see if the supplied file-lock exists
	static bool exists(const string& path);

	// Check to see if the supplied file-lock exists
	inline static bool exists(const char* path) {
		return exists(string(path));
	}

	// Retrieves the path to the file-lock.
	inline const string& path() const { return mPath; }

	// Check to see if this file-lock exists
	bool exists() const;

private:
	mutex mMutex;
	string mPath;
	atomic<uint32_t> mCount;
};
