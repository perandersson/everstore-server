#ifndef _EVERSTORE_TRANSACTION_H_
#define _EVERSTORE_TRANSACTION_H_

#include "es_config.h"
#include "Event.h"
#include "Bytes.h"
#include "TransactionId.h"

typedef uint64_t bit_mask;

typedef bit_mask transaction_types;

struct Journal;

/**
 * Name of the transaction type used for creating a new journal
 */
extern const string NEW_JOURNAL_TRANSACTION_TYPE;

/**
 * Bit mask for  of the transaction type used for creating a new journal
 */
const bit_mask NEW_JOURNAL_TRANSACTION_TYPE_BIT = BIT(0);

class Transaction
{
public:
	Transaction(const TransactionId id, Journal* journal);

	~Transaction();

	/**
	 * Commit this transaction and save it to the HDD
	 *
	 * @param events The entire transaction content
	 */
	void save(IntrusiveBytesString events);

	/**
	 * @return The transaction id
	 */
	inline TransactionId id() const { return mId; }

	/**
	 * Check to see if this transaction conflicts with any of the supplied types
	 *
	 * @param types A bit-mask field that represents a list of all transaction types we want to commit
	 * @return <code>true</code> if this commit indicates a conflict
	 */
	bool conflictsWith(transaction_types types) const;

	/**
	 * @param types A bit-mask field that represents a list of all transaction types we want to commit
	 */
	void onTransactionCommitted(transaction_types types);

	/**
	 * @return The size of the journal from this transaction's point-of-view
	 */
	inline uint32_t journalSize() const { return mJournalSize; }

	/**
	 * @return <code>true</code> if this transaction indicates that the journal should be created on commit
	 */
	inline bool createJournal() const { return mJournalSize == 0; }

private:
	const TransactionId mId;
	Journal* mJournal;
	uint32_t mJournalSize;
	transaction_types mTransactionTypesBeforeCommit;
};

#endif
