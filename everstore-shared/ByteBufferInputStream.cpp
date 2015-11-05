#include "ByteBufferInputStream.h"

ByteBufferInputStream::ByteBufferInputStream(Bytes* buffer) {
	mStart = buffer->ptr();
	buffer->memorize();
	buffer->moveFromEnd(0);
	mEnd = buffer->end() - 1;
	buffer->reset();

	assert(mStart != nullptr);
	assert(mEnd != nullptr);
}

ByteBufferInputStream::~ByteBufferInputStream() {

}

int32_t ByteBufferInputStream::lastIndexOf(char c) {
	while (mEnd != mStart) {
		if (*mEnd-- == c) {
			return mEnd - mStart + 1;
		}
	}
	return -1;
}
