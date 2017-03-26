#pragma once

#include "StringUtils.h"
#include <cassert>

/**
 * Type that represents a single process
 */
struct ChildProcessId
{
	const uint32_t value;

	ChildProcessId(const uint32_t value) : value(value) {
		assert(value != 0);
	}

	ChildProcessId(const ChildProcessId& rhs) : value(rhs.value) {}

	const bool operator!=(const ChildProcessId& rhs) const { return value != rhs.value; }

	const bool operator==(const ChildProcessId& rhs) const { return value == rhs.value; }

	/**
	 * Stringify this object
	 *
	 * @return A string that represents this object
	 */
	inline const string toString() const {
		return StringUtils::toString(value);
	}
};
