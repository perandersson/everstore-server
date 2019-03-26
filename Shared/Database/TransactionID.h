#ifndef _TRANSACTION_ID_H_
#define _TRANSACTION_ID_H_

#include "../es_config.h"

//
struct TransactionID {
	const uint32_t value;

	TransactionID(const uint32_t value) :value(value) {}

	const bool operator != (const TransactionID& rhs) const { return value != rhs.value; }
	const bool operator == (const TransactionID& rhs) const { return value == rhs.value; }
};

#endif
