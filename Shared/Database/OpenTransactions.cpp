//
// Copyright (c) 2019 West Coast Code AB. All rights reserved.
//

#include "OpenTransactions.hpp"

OpenTransactions::OpenTransactions() {

}

OpenTransactions::~OpenTransactions() {
	for (auto t : mTransactions) {
		delete t;
	}
	mTransactions.clear();
}

TransactionID OpenTransactions::open(Journal* journal) {
	// Try to reuse a transaction id
	const uint32_t num = mTransactions.size();
	for (uint32_t i = 0; i < num; ++i) {
		auto t = mTransactions[i];
		if (t == nullptr) {
			t = new Transaction(i, journal);
			mTransactions[i] = t;
			return i;
		}
	}

	// Create a new transaction id
	const auto id = mTransactions.size();
	Transaction* t = new Transaction(id, journal);
	mTransactions.push_back(t);
	return id;
}

Transaction* OpenTransactions::get(TransactionID id) {
	const auto value = id.value;

	// Ensure that we do not try to open a non-existing transaction
	if (value >= mTransactions.size()) return nullptr;

	// Make sure that the transaction actually matches what we are requesting
	return mTransactions[value];
}

void OpenTransactions::close(TransactionID id) {
	const auto value = id.value;

	// Ensure that we do not try to close a non-existing transaction
	if (value >= mTransactions.size()) return;

	// Remove the memory associated with the transaction
	auto const t = mTransactions[value];
	if (t) {
		delete t;
		mTransactions[value] = nullptr;
	}
}

void OpenTransactions::onTransactionCommitted(transaction_types types) {
	for (auto trans : mTransactions) {
		if (trans != nullptr) {
			trans->onTransactionCommitted(types);
		}
	}
}
