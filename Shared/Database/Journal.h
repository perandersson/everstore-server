#ifndef _EVERSTORE_JOURNAL_H_
#define _EVERSTORE_JOURNAL_H_

#include "../es_config.h"
#include "../Message/ESHeader.h"
#include "../File/FileLock.h"
#include "Transaction.h"
#include "../LinkedList.h"
#include "../File/FileInputStream.h"
#include "../File/FileOutputStream.h"
#include "../Ipc/ChildProcessID.h"
#include "OpenTransactions.hpp"

class Journal
{
public:
	// Use the nullptr character '\0' as an EOF marker
	static constexpr char JournalEof = '\0';

	// The length of the EOF character
	static constexpr auto JournalEofLen = 1;

	LinkedListLink<Journal> link;

	Journal(const string& path);

	Journal(const string& path, const ChildProcessID childProcessId);

	~Journal();

	// Perform consistency check on this journal
	bool performConsistencyCheck();

	// Refresh the life-time timestamp of this object
	void refresh();

	// Open a new transaction and return a unique id for it
	const TransactionID openTransaction();

	// Rollback the supplied transaction
	void rollback(const TransactionID id);

	// Try to commit a transaction
	ESErrorCode tryCommit(const TransactionID id, Bits::Type types, MutableString eventsString);

	// Increase the reference count of this journal and returns the size of the journal
	//
	// \return The size of the journal
	uint32_t addRef();

	// Release a reference counter
	//
	// \param bytesWritten The amount of bytes written to the journal
	void release(uint32_t bytesWritten);

	// Open a file input stream to the journal
	FileInputStream* inputStream(uint32_t bytesOffset);

	// Open a file output stream
	FileOutputStream* outputStream();

	// Open a file output stream
	FileOutputStream* outputStream(uint32_t bytesOffset);

	// Retrieves the size of this journal in bytes. The size of the journal might or might not be the same size as the journal file's size
	inline uint32_t journalSize() const { return mJournalSize; }

	// Is this journal empty
	inline bool empty() const { return mJournalSize == 0; }

	// Does this journal exists?
	inline bool exists() const { return !empty(); }

	// Retrieves the path to the journal (on the HDD)
	inline const string& name() const { return mName; }

	// Retrieves the path to the journal (on the HDD)
	inline const string& path() const { return mJournalFileName; }

	// When was the journal used last?
	inline const chrono::system_clock::time_point& timeSinceLastUsed() const { return mTimeSinceLastUsed; }

private:
	string mName;
	string mJournalFileName;
	FileLock mFileLock;
	chrono::system_clock::time_point mTimeSinceLastUsed;
	uint32_t mJournalSize;
	OpenTransactions mTransactions;


};


#endif
