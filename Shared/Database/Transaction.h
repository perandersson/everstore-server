#ifndef _EVERSTORE_TRANSACTION_H_
#define _EVERSTORE_TRANSACTION_H_

#include "../es_config.h"
#include "../Event.h"
#include "../Memory/ByteBuffer.h"
#include "TransactionID.h"

typedef bit_mask transaction_types;

struct Journal;

// Transaction type used to indicate that a journal is new
extern const string NEW_JOURNAL_TRANSACTION_TYPE;

// Transaction bit mask for new journals
const bit_mask NEW_JOURNAL_TRANSACTION_TYPE_BIT = BIT(0);

struct Transaction {

	Transaction(TransactionID id, Journal* journal);

	~Transaction();

	// Commit this transaction and save it to the HDD in a way that can be fixed if the worker closes ungracefully
	void save(IntrusiveBytesString events);

	// Retrieves the transaction id
	inline const TransactionID id() const { return mId; }

	// Check if this transaction conflicts with any of the supplied types
	bool conflictsWith(transaction_types types) const;

	// Method called when a transaction is committed on the same journal
	void onTransactionCommitted(transaction_types types);

	// Retrieves the journal size from this transactions view-point
	inline uint32_t journalSize() const { return mJournalSize; }

	// Does this transaction indicate that the journal will be created on commit
	inline bool createJournal() const { return mJournalSize == 0; }

private:
	const TransactionID mId;
	Journal* mJournal;
	uint32_t mJournalSize;
	transaction_types mTransactionTypesBeforeCommit;
};

#endif
