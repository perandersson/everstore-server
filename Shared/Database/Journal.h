#ifndef _EVERSTORE_JOURNAL_H_
#define _EVERSTORE_JOURNAL_H_

#include "../es_config.h"
#include "../Message/ESHeader.h"
#include "../File/FileLock.h"
#include "Transaction.h"
#include "../LinkedList.h"
#include "../File/FileInputStream.h"
#include "../File/FileOutputStream.h"
#include "OpenTransactions.hpp"
#include "../File/Path.hpp"

class Journal
{
public:
	// Use the nullptr character '\0' as an EOF marker
	static constexpr char JournalEof = '\0';

	// The length of the EOF character
	static constexpr auto JournalEofLen = 1;

	LinkedListLink<Journal> link;

	Journal(const Path& path);

	Journal(const Path& path, ProcessID childProcessId);

	~Journal();

	// Perform consistency check on this journal
	bool performConsistencyCheck();

	// Refresh the life-time timestamp of this object
	void refresh();

	// Open a new transaction and return a unique id for it
	const TransactionID openTransaction();

	// Rollback the supplied transaction
	void rollback(TransactionID id);

	// Try to commit a transaction
	ESErrorCode tryCommit(TransactionID id, Bits::Type types, MutableString eventsString);

	// Increase the reference count of this journal and returns the size of the journal
	//
	// \return The size of the journal
	uint32_t addRef();

	// Release a reference counter
	//
	// \param bytesWritten The amount of bytes written to the journal
	void release(uint32_t bytesWritten);

	// Retrieves the size of this journal in bytes. The size of the journal might or might not be the same size as the journal file's size
	inline uint32_t journalSize() const { return mJournalSize; }

	// Is this journal empty
	inline bool empty() const { return mJournalSize == 0; }

	// Does this journal exists?
	inline bool exists() const { return !empty(); }

	// Retrieves the path to the journal (on the HDD)
	inline const Path& path() const { return mPath; }

	inline FILE* file() const { return mFile; }

	// When was the journal used last?
	inline const chrono::system_clock::time_point& timeSinceLastUsed() const { return mTimeSinceLastUsed; }

	// Open a file input stream to the journal
	FileInputStream* inputStream(uint32_t bytesOffset);

	// Open a file output stream
	FileOutputStream* outputStream();

	// Open a file output stream
	FileOutputStream* outputStream(uint32_t bytesOffset);

private:
	// The path to this journal
	const Path mPath;

	// Points to the actual file on the hdd
	FILE* const mFile;

	FileLock mFileLock;
	chrono::system_clock::time_point mTimeSinceLastUsed;
	uint32_t mJournalSize;
	OpenTransactions mTransactions;


};


#endif
