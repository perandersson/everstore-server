#pragma once

#include "StringUtils.h"
#include <cassert>

/**
 * Type that represents a transaction
 */
struct TransactionId
{
	const uint32_t value;

	TransactionId(const uint32_t value) : value(value) {
		assert(value != 0);
	}

	TransactionId(const TransactionId& rhs) : value(rhs.value) {}

	const bool operator!=(const TransactionId& rhs) const { return value != rhs.value; }

	const bool operator==(const TransactionId& rhs) const { return value == rhs.value; }

	/**
	 * Stringify this object
	 *
	 * @return A string that represents this object
	 */
	inline const string toString() const {
		return StringUtils::toString(value);
	}
};
