#include "ByteBufferInputStream.h"
#include <cassert>

ByteBufferInputStream::ByteBufferInputStream(ByteBuffer* buffer) {
	mStart = buffer->ptr();
	buffer->memorize();
	buffer->moveFromEnd(0);
	mEnd = buffer->end() - 1;
	buffer->reset();

	assert(mStart != nullptr);
	assert(mEnd != nullptr);
}

int32_t ByteBufferInputStream::lastIndexOf(char c) {
	while (mEnd != mStart) {
		if (*mEnd-- == c) {
			return mEnd - mStart + 1;
		}
	}
	return -1;
}
