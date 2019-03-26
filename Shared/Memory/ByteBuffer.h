#ifndef _EVERSTORE_BYTES_H_
#define _EVERSTORE_BYTES_H_

#include "../es_config.h"

//
// Structure representing raw memory
struct ByteBuffer {

	ByteBuffer(uint32_t initialSize);

	~ByteBuffer();

	void moveForward(uint32_t offset);

	void moveBackwards(uint32_t offset);

	void moveFromStart(uint32_t offset);

	void moveFromEnd(uint32_t offset);

	// Ensure that this memory block has enough capacity.
	void ensureCapacity(uint32_t size);

	inline void reset() { mCurrentOffset = 0; }

	// Memorize the current position in memory. Use restore to go back to where the memorized location.
	inline void memorize() { mSavedOffset = mCurrentOffset; }

	// Restore the position into when the memorize method was called.
	inline void restore() { mCurrentOffset = mSavedOffset; }

	// Put the supplied memory into this bytes block
	void put(const void* ptr, const uint32_t size);

	// Put the supplied type into this bytes block
	template<typename T>
	void put(T* ptr) {
		put(ptr, sizeof(T));
	}

	// Retrieves a memory block with large enough for the requested size
	inline char* get(const uint32_t size) {
		ensureCapacity(size);
		char* ptr = &mMemory[mCurrentOffset];
		moveForward(size);
		return ptr;
	}

	// Retrieves a memory block with large enough for the requested type
	template<typename T>
	T* get() {
		return (T*)get(sizeof(T));
	}

	inline uint32_t capacity() const { return mCapacity; }

	inline uint32_t offset() const { return mCurrentOffset; }

	inline const char* ptr() const { return mMemory; }

	inline char* ptr() { return mMemory; }

	inline const char* end() const { return &mMemory[mCurrentOffset]; }

private:
	// Clamp the supplied memory position to the size of the buffer to ensure
	// that we do not exceed it.
	inline uint32_t clamp(uint32_t pos) {
		if (pos > mCapacity) pos = offsetToEnd();
		return pos;
	}

	inline uint32_t offsetToEnd() const { return mCapacity; }

private:
	char* mMemory;
	uint32_t mCapacity;
	uint32_t mCurrentOffset;
	uint32_t mSavedOffset;
};

//
// Intrusive string directly connected to it's associated ByteBuffer.
//
// \remark This might become invalidated when the associated ByteBuffer is resetted.
//
struct IntrusiveBytesString {
	uint32_t length; // The length of the string.
	const char* str; // A pointer to the first character in the string. The string itself might not end with NULL.

	IntrusiveBytesString(uint32_t length, ByteBuffer* b) : length(length), str(b->get(length)) {}
	~IntrusiveBytesString() {}
};

#endif
