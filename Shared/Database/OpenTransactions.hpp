//
// Copyright (c) 2019 West Coast Code AB. All rights reserved.
//

#ifndef EVERSTORE_OPENTRANSACTIONS_HPP
#define EVERSTORE_OPENTRANSACTIONS_HPP

#include "Transaction.h"

class OpenTransactions
{
public:
	OpenTransactions();

	~OpenTransactions();

	// Open a new transaction for the supplied journal
	//
	// \param journal
	// \return A unique ID for the journal
	TransactionID open(Journal* journal);

	// Retrieve the transaction with the given id
	Transaction* get(TransactionID id);

	// Close the supplied transaction
	void close(TransactionID id);

	/**
	 * Method called whenever a new transaction is committed
	 */
	void onTransactionCommitted(Bits::Type changes);


private:
	vector<Transaction*> mTransactions;
};


#endif //EVERSTORE_OPENTRANSACTIONS_HPP
