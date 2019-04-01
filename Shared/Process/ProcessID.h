#ifndef _EVERSTORE_PROCESS_ID_H_
#define _EVERSTORE_PROCESS_ID_H_

#include "../es_config.h"
#include "../StringUtils.h"

//
struct ProcessID
{
	const uint32_t value;

	explicit ProcessID(const uint32_t value)
			: value(value) {
	}

	ProcessID(const ProcessID& rhs) = default;

	inline bool operator!=(const ProcessID& rhs) const { return value != rhs.value; }

	inline bool operator==(const ProcessID& rhs) const { return value == rhs.value; }

	inline uint32_t AsIndex() const { return value - 1u; }

	inline bool IsValid() const { return value > 0u; }

	inline bool IsInvalid() const { return value == 0u; }

	// Converts this id value to a string
	inline const string ToString() const { return StringUtils::toString(value); }
};

static_assert(sizeof(ProcessID) == 4, "Expected the ProcessID type to be 4 bytes");

#endif
