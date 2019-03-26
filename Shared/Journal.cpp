#include "Journal.h"
#include "File/FileUtils.h"
#include "StringUtils.h"
#include "Transaction.h"
#include "Memory/ByteBufferInputStream.h"
#include "AutoClosable.h"

OpenTransactions::OpenTransactions() {

}

OpenTransactions::~OpenTransactions() {
	for (auto t : *this) {
		delete t;
	}
	clear();
}

const TransactionId OpenTransactions::open(Journal* journal) {
	// Try to reuse a transaction id
	const uint32_t num = size();
	for (uint32_t i = 0; i < num; ++i) {
		auto t = operator[](i);
		if (t == nullptr) {
			Transaction* t = new Transaction(i, journal);
			operator[](i) = t;
			return i;
		}
	}

	// Create a new transaction id
	const TransactionId id = size();
	Transaction* t = new Transaction(id, journal);
	push_back(t);
	return id;
}

Transaction* OpenTransactions::get(const TransactionId id) {
	const auto value = id.value;

	// Ensure that we do not try to open a non-existing transaction
	if (value >= size()) return nullptr;

	// Make sure that the transaction actually matches what we are requesting
	auto t = operator[](value);
	return t;
}

void OpenTransactions::close(const TransactionId id) {
	const auto value = id.value;

	// Ensure that we do not try to close a non-existing transaction
	if (value >= size()) return;

	// Remove the memory associated with the transaction
	auto t = operator[](value);
	if (t != nullptr) {
		delete t;
		operator[](value) = nullptr;
	}
}

Journal::Journal(const string& name) : mName(name), mJournalFileName(name + string(".log")),
mFileLock(name + string(".lock")),
mTimeSinceLastUsed(chrono::system_clock::now()),
mJournalSize(0),
mTransactions() {
	// The journal size is assumed to be the file size. Only one journal instance can exists for the same file and
	// since the consistency check is done before, then the file size is the same as the journal size
	mJournalSize = FileUtils::getFileSize(mJournalFileName);
}

Journal::Journal(const string& name, ChildProcessId workerId) :
mName(name), mJournalFileName(name + string(".log")),
mFileLock(name + string(".") + workerId.toString() + string(".lock")),
mTimeSinceLastUsed(chrono::system_clock::now()),
mJournalSize(0),
mTransactions() {
	// The journal size is assumed to be the file size. Only one journal instance can exists for the same file and
	// since the consistency check is done before, then the file size is the same as the journal size
	mJournalSize = FileUtils::getFileSize(mJournalFileName);
}

Journal::~Journal() {
}

bool Journal::performConsistencyCheck() {
	// Lock file still exists, which means that this journals might be inconsistent.
	if (exists() && mFileLock.exists()) {
		
		// Open and read the journal file into memory
		Bytes buffer(mJournalSize);
		ESErrorCode err = AutoClosable<FileInputStream>(inputStream(0))->readBytes(&buffer);
		if (err != ESERR_NO_ERROR) return false;

		// Open a new stream for the memory buffer
		ByteBufferInputStream stream(&buffer);

		// Search for the eof marker
		const int32_t eof = stream.lastIndexOf(JOURNAL_EOF);

		// If the last character is not an EOF-marker then it indicates that we have an unfinished transaction
		if (eof == -1) {
			// We've tried to save our first transaction but failed. Remove the entire file
			auto result = ::remove(mJournalFileName.c_str());
			if (result != 0) return false;
			mJournalSize = 0;
		}
		else if ((uint32_t)eof == mJournalSize - 1) {
			// If the last character is a EOF-marker then crash occurred when lock file is being created or being removed.
			// I.e. we don't have to do anything, or we might have to search for the next marker

			// Find where the EOF-marker might be
			const auto potentialNextEof = stream.lastIndexOf(JOURNAL_EOF);

			// No EOF marker found then the journal is safe. The only thing that was missing was to remove the lock file
			if (potentialNextEof == -1) {
				auto result = remove(mFileLock.path().c_str());
				return result == 0;
			}

			// Another EOF marker was found, then all the necessary data was saved in the journal, but the removal of the
			// eof-marker at the start of the transaction is still there. Remove it and we are safe
			auto writer = outputStream();
			writer->replaceWithNL(potentialNextEof);
			writer->close();
		} else {
			// Remove the unfinished written transaction from the journal file
			FileUtils::truncate(mJournalFileName, eof + 1);
			mJournalSize = (uint32_t)eof + 1;
		}

		auto result = remove(mFileLock.path().c_str());
		return result == 0;
	}

	return true;
}

void Journal::refresh() {
	mTimeSinceLastUsed = chrono::system_clock::now();
}

const TransactionId Journal::openTransaction() {
	return mTransactions.open(this);
}

void Journal::rollback(const TransactionId id) {
	mTransactions.close(id);
}

ESErrorCode Journal::tryCommit(const TransactionId id, transaction_types types, IntrusiveBytesString eventsString) {
	// Retrieve the active transaction
	auto t = mTransactions.get(id);
	if (t == nullptr) return ESERR_JOURNAL_TRANSACTION_DOES_NOT_EXIST;

	// Set neccessary bit if the journal is to be created
	if (t->createJournal()) {
		FileUtils::createFullForPath(path());
		BIT_SET(types, NEW_JOURNAL_TRANSACTION_TYPE_BIT);
	}

	// Has a conflict occured?
	if (t->conflictsWith(types)) {
		return ESERR_JOURNAL_TRANSACTION_CONFLICT;
	}

	// Commit the transaction and close it from being open
	t->save(eventsString);
	mTransactions.close(id);

	// Notify all open transactions that another transaction has been committed
	for (auto trans : mTransactions) {
		if (trans != nullptr) {
			trans->onTransactionCommitted(types);
		}
	}

	return ESERR_NO_ERROR;
}

uint32_t Journal::addRef() {
	mFileLock.addRef();
	return mJournalSize;
}

void Journal::release(uint32_t bytesWritten) {
	mFileLock.release();
	mJournalSize += bytesWritten;
}

FileInputStream* Journal::inputStream(uint32_t bytesOffset) {
	return new FileInputStream(mJournalFileName, mJournalSize, bytesOffset);
}

FileOutputStream* Journal::outputStream() {
	return new FileOutputStream(mJournalFileName, 0);
}

FileOutputStream* Journal::outputStream(uint32_t bytesOffset) {
	bytesOffset = bytesOffset > mJournalSize ? mJournalSize : bytesOffset;
	return new FileOutputStream(mJournalFileName, bytesOffset);
}
