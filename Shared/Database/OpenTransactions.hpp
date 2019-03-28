//
// Copyright (c) 2019 West Coast Code AB. All rights reserved.
//

#ifndef EVERSTORE_OPENTRANSACTIONS_HPP
#define EVERSTORE_OPENTRANSACTIONS_HPP

#include "Transaction.h"

/**
 * Type that helps us keeping track of all opened transactions
 */
class OpenTransactions
{
public:
	OpenTransactions() = default;

	~OpenTransactions();

	/**
	 * Open a new transaction for the supplied journal
	 *
	 * @param journal
	 * @return A unique ID for the journal that represents this transaction
	 */
	TransactionID open(Journal* journal);

	/**
	 * Retrieve the transaction with the given id
	 *
	 * @param id The transaction ID
	 * @return The transaction if found; <code>nullptr</code> otherwise
	 */
	Transaction* get(TransactionID id);

	/**
	 * Close the supplied transaction
	 *
	 * @param id The transaction ID
	 */
	void close(TransactionID id);

	/**
	 * Method called whenever a new transaction is committed
	 */
	void onTransactionCommitted(Bits::Type changes);

private:
	vector<Transaction*> mTransactions;
};


#endif //EVERSTORE_OPENTRANSACTIONS_HPP
