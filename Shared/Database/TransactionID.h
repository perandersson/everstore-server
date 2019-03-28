#ifndef _TRANSACTION_ID_H_
#define _TRANSACTION_ID_H_

#include <cinttypes>

/**
 * Represents the ID for a specific transaction
 */
struct TransactionID
{
	const uint32_t value;

	explicit TransactionID(const uint32_t value)
			: value(value) {
	}

	TransactionID(const TransactionID& rhs) = default;

	inline uint32_t AsIndex() const { return value - 1u; }

	inline bool IsValid() const { return value > 0; }

	inline bool IsInvalid() const { return value == 0; }

	inline bool operator!=(const TransactionID& rhs) const { return value != rhs.value; }

	inline bool operator==(const TransactionID& rhs) const { return value == rhs.value; }
};

static_assert(sizeof(TransactionID) == 4, "TransactionID is assumed to be 4 bytes in size");

#endif
