#ifndef _EVERSTORE_JOURNAL_H_
#define _EVERSTORE_JOURNAL_H_

#include "es_config.h"
#include "ESHeader.h"
#include "FileLock.h"
#include "Transaction.h"
#include "LinkedList.h"
#include "FileInputStream.h"
#include "FileOutputStream.h"
#include "ChildProcessId.h"

static const char JOURNAL_EOF = 0; // Use NULL as EOF marker
static const uint32_t JOURNAL_EOF_LEN = 1;

struct OpenTransactions : vector<Transaction*> {

	OpenTransactions();

	~OpenTransactions();

	// Open a new transaction for the supplied journal
	// 
	// \param journal
	// \return A unique ID for the journal
	const TransactionId open(Journal* journal);

	// Retrieve the transaction with the given id
	Transaction* get(const TransactionId id);

	// Close the supplied transaction
	void close(const TransactionId id);
};

struct Journal {
	LinkedListLink<Journal> link;

	Journal(const string& path); 

	Journal(const string& path, const ChildProcessId childProcessId);

	~Journal();

	// Perform consistency check on this journal
	bool performConsistencyCheck();

	// Refresh the life-time timestamp of this object
	void refresh();

	// Open a new transaction and return a unique id for it
	const TransactionId openTransaction();

	// Rollback the supplied transaction
	void rollback(const TransactionId id);

	// Try to commit a transaction
	ESErrorCode tryCommit(const TransactionId id, transaction_types types, IntrusiveBytesString eventsString);

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
	chrono::system_clock::time_point  mTimeSinceLastUsed;
	uint32_t mJournalSize;
	OpenTransactions mTransactions;

	
};


#endif
