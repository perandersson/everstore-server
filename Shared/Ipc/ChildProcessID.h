#ifndef _CHILD_PROCESS_ID_H_
#define _CHILD_PROCESS_ID_H_

#include "../es_config.h"
#include "../StringUtils.h"

//
struct ChildProcessID
{
	const uint32_t value;

	explicit ChildProcessID(const uint32_t value)
			: value(value) {
		assert(value != 0);
	}

	ChildProcessID(const ChildProcessID& rhs) = default;

	const bool operator!=(const ChildProcessID& rhs) const { return value != rhs.value; }

	const bool operator==(const ChildProcessID& rhs) const { return value == rhs.value; }

	inline uint32_t asIndex() const { return value - 1u; }

	// Converts this id value to a string
	inline const string toString() const { return StringUtils::toString(value); }
};

#endif
