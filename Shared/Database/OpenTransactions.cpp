//
// Copyright (c) 2019 West Coast Code AB. All rights reserved.
//

#include "OpenTransactions.hpp"
#include "Journal.h"

OpenTransactions::~OpenTransactions() {
	for (auto t : mTransactions) {
		delete t;
	}
	mTransactions.clear();
}

TransactionID OpenTransactions::open(Journal* journal) {
	auto const file = journal->file();
	if (!file) {
		return TransactionID(0);
	}

	// Try to reuse a transaction id
	const uint32_t num = mTransactions.size();
	for (uint32_t i = 0; i < num; ++i) {
		auto t = mTransactions[i];
		if (t == nullptr) {
			const TransactionID id(i + 1u);
			t = new Transaction(id, file, journal);
			mTransactions[i] = t;
			return id;
		}
	}

	// Create a new transaction id
	const TransactionID id(num + 1u);
	auto const t = new Transaction(id, file, journal);
	mTransactions.push_back(t);
	return id;
}

Transaction* OpenTransactions::get(TransactionID id) {
	// Only return valid transactions
	if (id.IsInvalid()) {
		return nullptr;
	}

	// Ensure that we do not try to open a non-existing transaction
	const auto value = id.AsIndex();
	if (value >= mTransactions.size()) {
		return nullptr;
	}

	// Make sure that the transaction actually matches what we are requesting
	return mTransactions[value];
}

void OpenTransactions::close(TransactionID id) {
	// Only valid transactions
	if (id.IsInvalid()) {
		return;
	}

	// Ensure that we do not try to close a non-existing transaction
	const auto value = id.AsIndex();
	if (value >= mTransactions.size()) {
		return;
	}

	// Remove the memory associated with the transaction
	auto const t = mTransactions[value];
	if (t) {
		delete t;
		mTransactions[value] = nullptr;
	}
}

void OpenTransactions::onTransactionCommitted(Bits::Type changes) {
	for (auto trans : mTransactions) {
		if (trans != nullptr) {
			trans->onTransactionCommitted(changes);
		}
	}
}
