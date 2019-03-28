#include "Transaction.h"
#include "Journal.h"

Transaction::Transaction(TransactionID id, FILE* file, Journal* journal)
		: mId(id), mFile(file), mJournal(journal),
		  mTransactionTypesBeforeCommit(Bits::None) {
	mJournalSize = FileUtils::getFileSize(file);
}

void Transaction::save(MutableString eventsString) {
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
	delete writer;

	// We are now done with accessing the journal on disk
	mJournal->release(bytesWritten);
}
