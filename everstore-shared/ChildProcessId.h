#ifndef _CHILD_PROCESS_ID_H_
#define _CHILD_PROCESS_ID_H_

#include "es_config.h"
#include "StringUtils.h"

//
struct ChildProcessId {
	const uint32_t value;

	ChildProcessId(const uint32_t value) :value(value) {
		assert(value != 0);
	}

	const bool operator != (const ChildProcessId& rhs) const { return value != rhs.value; }
	const bool operator == (const ChildProcessId& rhs) const { return value == rhs.value; }

	// Converts this id value to a string
	inline const string toString() const { return StringUtils::toString(value); }
};

#endif
