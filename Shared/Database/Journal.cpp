#include "Journal.h"
#include "../File/FileUtils.h"
#include "../StringUtils.h"
#include "Transaction.h"
#include "../Memory/ByteBufferInputStream.h"
#include "../AutoClosable.h"

Journal::Journal(const Path& path)
		: mPath(path),
		  mFile(path.OpenOrCreate("r+b")),
		  mFileLock(path.value + string(".lock")),
		  mTimeSinceLastUsed(chrono::system_clock::now()),
		  mJournalSize(0) {
	// The journal size is assumed to be the file size. Only one journal instance can exists for the same file and
	// since the consistency check is done before, then the file size is the same as the journal size
	mJournalSize = FileUtils::getFileSize(mFile);
}

Journal::Journal(const Path& path, ChildProcessID workerId) :
		mPath(path),
		mFile(path.OpenOrCreate("r+b")),
		mFileLock(path.value + string(".") + workerId.toString() + string(".lock")),
		mTimeSinceLastUsed(chrono::system_clock::now()),
		mJournalSize(0) {
	// The journal size is assumed to be the file size. Only one journal instance can exists for the same file and
	// since the consistency check is done before, then the file size is the same as the journal size
	mJournalSize = FileUtils::getFileSize(mFile);
}

Journal::~Journal() {
	if (mFile) {
		fclose(mFile);
	}
}

bool Journal::performConsistencyCheck() {
	// Ignore if no lock file exists
	if (!exists() || !mFileLock.exists()) {
		return true;
	}

	// Open and read the journal file into memory
	ByteBuffer buffer(mJournalSize);
	std::shared_ptr<FileInputStream> inputStream(Journal::inputStream(0u));
	auto err = inputStream->readBytes(&buffer);
	if (err != ESERR_NO_ERROR) {
		return false;
	}

	// Open a new stream for the memory buffer
	ByteBufferInputStream stream(&buffer);

	// Search for the eof marker
	const int32_t eof = stream.lastIndexOf(Journal::JournalEof);

	// If the last character is not an EOF-marker then it indicates that we have an unfinished transaction
	if (eof == -1) {
		// We've tried to save our first transaction but failed. Remove the entire file
		auto result = FileUtils::truncate(mFile, 0u);
		if (!result)
			return false;
		mJournalSize = 0;
	} else if ((uint32_t) eof == mJournalSize - 1) {
		// If the last character is a EOF-marker then crash occurred when lock file is being created or being removed.
		// I.e. we don't have to do anything, or we might have to search for the next marker

		// Find where the EOF-marker might be
		const auto potentialNextEof = stream.lastIndexOf(Journal::JournalEof);

		// No EOF marker found then the journal is safe. The only thing that was missing was to remove the lock file
		if (potentialNextEof == -1) {
			auto result = remove(mFileLock.path().c_str());
			return result == 0;
		}

		// Another EOF marker was found, then all the necessary data was saved in the journal, but the removal of the
		// eof-marker at the start of the transaction is still there. Remove it and we are safe
		auto writer = std::shared_ptr<FileOutputStream>(outputStream());
		writer->replaceWithNL(potentialNextEof);
	} else {
		// Remove the unfinished written transaction from the journal file
		FileUtils::truncate(mPath.value, eof + 1);
		mJournalSize = (uint32_t) eof + 1;
	}

	// Remove the file lock when we are done
	auto result = remove(mFileLock.path().c_str());
	return result == 0;
}

void Journal::refresh() {
	mTimeSinceLastUsed = chrono::system_clock::now();
}

const TransactionID Journal::openTransaction() {
	return mTransactions.open(this);
}

void Journal::rollback(TransactionID id) {
	mTransactions.close(id);
}

ESErrorCode Journal::tryCommit(TransactionID id, Bits::Type types, MutableString eventsString) {
	// Retrieve the active transaction
	auto t = mTransactions.get(id);
	if (t == nullptr) return ESERR_JOURNAL_TRANSACTION_DOES_NOT_EXIST;

	// Set neccessary bit if the journal is to be created
	if (t->createJournal()) {
		FileUtils::createFullForPath(mPath.value);
		types = Bits::Set(types, Bits::BuiltIn::NewJournalBit);
	}

	// Has a conflict occured?
	if (t->conflictsWith(types)) {
		return ESERR_JOURNAL_TRANSACTION_CONFLICT;
	}

	// Commit the transaction and close it from being open
	t->save(eventsString);
	mTransactions.close(id);

	// Notify all open transactions that another transaction has been committed
	mTransactions.onTransactionCommitted(types);

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
	return new FileInputStream(mFile, mJournalSize, bytesOffset);
}

FileOutputStream* Journal::outputStream() {
	return new FileOutputStream(mFile, 0);
}

FileOutputStream* Journal::outputStream(uint32_t bytesOffset) {
	bytesOffset = bytesOffset > mJournalSize ? mJournalSize : bytesOffset;
	return new FileOutputStream(mFile, bytesOffset);
}
