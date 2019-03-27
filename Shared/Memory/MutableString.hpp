//
// Copyright (c) 2019 West Coast Code AB. All rights reserved.
//

#ifndef EVERSTORE_MUTABLESTRING_HPP
#define EVERSTORE_MUTABLESTRING_HPP

#include "ByteBuffer.h"

/**
 * A very special type that represents a string. One important aspect of this is that it's tightly connected
 * to the underlying Bytes structure.
 *
 * @remark If the underlying memory is reset then this string will be invalidated.
 */
struct MutableString
{
	// The length of the string
	const uint32_t length;

	// A pointer to the first character in the string. The string itself might not end with NULL.
	const char* str;

	MutableString(uint32_t length, ByteBuffer* b)
			: length(length), str(b->get(length)) {
	}

	MutableString(const MutableString& rhs) = default;
};

#endif //EVERSTORE_MUTABLESTRING_HPP
