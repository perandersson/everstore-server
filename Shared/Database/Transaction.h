#ifndef _EVERSTORE_TRANSACTION_H_
#define _EVERSTORE_TRANSACTION_H_

#include "../es_config.h"
#include "../Event.h"
#include "../Memory/ByteBuffer.h"
#include "TransactionID.h"
#include "../Bits.hpp"
#include "../Memory/MutableString.hpp"

struct Journal;

class Transaction
{
public:
	Transaction(TransactionID id, Journal* journal);

	// Commit this transaction and save it to the HDD in a way that can be fixed if the worker closes ungracefully
	void save(MutableString events);

	// Retrieves the transaction id
	inline const TransactionID id() const { return mId; }

	// Check if this transaction conflicts with any of the supplied types
	inline bool conflictsWith(Bits::Type types) const {
		return Bits::IsSet(mTransactionTypesBeforeCommit, types);
	}

	// Method called when a transaction is committed on the same journal
	inline void onTransactionCommitted(Bits::Type types) {
		mTransactionTypesBeforeCommit = Bits::Set(mTransactionTypesBeforeCommit, types);
	}

	// Retrieves the journal size from this transactions view-point
	inline uint32_t journalSize() const { return mJournalSize; }

	// Does this transaction indicate that the journal will be created on commit
	inline bool createJournal() const { return mJournalSize == 0; }

private:
	const TransactionID mId;
	Journal* mJournal;
	uint32_t mJournalSize;
	Bits::Type mTransactionTypesBeforeCommit;
};

#endif
