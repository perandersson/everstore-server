#include "Bytes.h"

Bytes::Bytes(uint32_t initialSize) : mMemory(nullptr), mCapacity(initialSize), mCurrentOffset(0), mSavedOffset(0) {
	assert(initialSize > 0);
	mMemory = (char*)malloc(mCapacity);
}

Bytes::~Bytes() {
	free(mMemory);
	mMemory = nullptr;
}

void Bytes::moveForward(uint32_t offset) {
	mCurrentOffset += offset;
	mCurrentOffset = clamp(mCurrentOffset);
}

void Bytes::moveBackwards(uint32_t offset) {
	if (offset > mCurrentOffset) mCurrentOffset = 0;
	else mCurrentOffset -= offset;
}

void Bytes::moveFromStart(uint32_t offset) {
	mCurrentOffset = offset;
	mCurrentOffset = clamp(mCurrentOffset);
}

void Bytes::moveFromEnd(uint32_t offset) {
	if (offset > offsetToEnd()) mCurrentOffset = 0;
	else mCurrentOffset = offsetToEnd() - offset;
}

void Bytes::ensureCapacity(uint32_t size) {
	const uint32_t requiredSize = mCurrentOffset + size;
	if (requiredSize > mCapacity) {
		mCapacity = requiredSize;
		mMemory = (char*)realloc(mMemory, mCapacity);
	}
}

void Bytes::put(const void* ptr, const uint32_t size) {
	ensureCapacity(size);
	memcpy(&mMemory[mCurrentOffset], ptr, size);
	moveForward(size);
}
