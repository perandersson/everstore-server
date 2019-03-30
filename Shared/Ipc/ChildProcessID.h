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
	}

	ChildProcessID(const ChildProcessID& rhs) = default;

	inline bool operator!=(const ChildProcessID& rhs) const { return value != rhs.value; }

	inline bool operator==(const ChildProcessID& rhs) const { return value == rhs.value; }

	inline uint32_t asIndex() const { return value - 1u; }

	inline bool isValid() const { return value > 0u; }

	inline bool isInvalid() const { return value == 0u; }

	// Converts this id value to a string
	inline const string toString() const { return StringUtils::toString(value); }
};

#endif
