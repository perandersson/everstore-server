//
// Copyright (c) 2019 West Coast Code AB. All rights reserved.
//

#ifndef EVERSTORE_PORT_HPP
#define EVERSTORE_PORT_HPP

#include <cinttypes>
#include "../StringUtils.h"

struct Port
{
	const uint32_t value;

	explicit Port(const uint32_t value)
			: value(value) {
	}

	Port(const Port& rhs) = default;

	inline bool operator!=(const Port& rhs) const { return value != rhs.value; }

	inline bool operator==(const Port& rhs) const { return value == rhs.value; }

	inline uint32_t AsIndex() const { return value - 1u; }

	inline bool IsValid() const { return value > 0u; }

	inline bool IsInvalid() const { return value == 0u; }

	// Converts this id value to a string
	inline const string ToString() const { return StringUtils::toString(value); }
};

#endif //EVERSTORE_PORT_HPP
