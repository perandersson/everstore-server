#include "Transaction.h"
#include "Journal.h"

const string NEW_JOURNAL_TRANSACTION_TYPE("ES_NewJournal");

Transaction::Transaction(const TransactionId id, Journal* journal) : mId(id), mJournal(journal), mTransactionTypesBeforeCommit(BIT_NONE) {
	mJournalSize = journal->journalSize();
}

Transaction::~Transaction() {
}

void Transaction::save(IntrusiveBytesString eventsString) {
	// Ignore if no events are to be committed
	if (eventsString.length == 0) return;

	// Increase reference counter to the journal
	auto fileSize = mJournal->addRef();

	// Open a stream that we can write to
	auto writer = mJournal->outputStream(fileSize);

	// Write the data onto the journal
	Timestamp now;
	uint32_t bytesWritten = writer->writeEvents(&now, eventsString);

	// Close the stream and flush the content to the disk
	writer->close();

	// We are now done with accessing the journal on disk
	mJournal->release(bytesWritten);
}

bool Transaction::conflictsWith(transaction_types types) const {
	return BIT_ISSET(mTransactionTypesBeforeCommit, types);
}

void Transaction::onTransactionCommitted(transaction_types types) {
	BIT_SET(mTransactionTypesBeforeCommit, types);
}
