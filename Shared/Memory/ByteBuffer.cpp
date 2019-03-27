#include "ByteBuffer.h"
#include <cassert>
#include <cstdlib>
#include <cstring>

ByteBuffer::ByteBuffer(uint32_t initialSize)
		: mMemory(nullptr), mCapacity(initialSize), mCurrentOffset(0), mSavedOffset(0) {
	assert(initialSize > 0);
	mMemory = (char*) malloc(mCapacity);
}

ByteBuffer::~ByteBuffer() {
	free(mMemory);
	mMemory = nullptr;
}

void ByteBuffer::moveForward(uint32_t offset) {
	mCurrentOffset += offset;
	mCurrentOffset = clamp(mCurrentOffset);
}

void ByteBuffer::moveBackwards(uint32_t offset) {
	if (offset > mCurrentOffset) mCurrentOffset = 0;
	else mCurrentOffset -= offset;
}

void ByteBuffer::moveFromStart(uint32_t offset) {
	mCurrentOffset = offset;
	mCurrentOffset = clamp(mCurrentOffset);
}

void ByteBuffer::moveFromEnd(uint32_t offset) {
	if (offset > offsetToEnd()) mCurrentOffset = 0;
	else mCurrentOffset = offsetToEnd() - offset;
}

void ByteBuffer::ensureCapacity(uint32_t size) {
	const uint32_t requiredSize = mCurrentOffset + size;
	if (requiredSize > mCapacity) {
		mCapacity = requiredSize;
		mMemory = (char*) realloc(mMemory, mCapacity);
	}
}

void ByteBuffer::write(const void* ptr, uint32_t size) {
	ensureCapacity(size);
	memcpy(&mMemory[mCurrentOffset], ptr, size);
	moveForward(size);
}

char* ByteBuffer::allocate(uint32_t size) {
	ensureCapacity(size);
	char* ptr = &mMemory[mCurrentOffset];
	moveForward(size);
	return ptr;
}
