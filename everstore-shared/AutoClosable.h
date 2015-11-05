#ifndef _EVERSTORE_AUTOCLOSABLE_H_
#define _EVERSTORE_AUTOCLOSABLE_H_

// Type to ensure that the supplied item calls it's "close" method when it goes out of scope
template<typename T>
struct AutoClosable {

	AutoClosable(T* ptr) : mPtr(ptr) {}

	~AutoClosable() {
		mPtr->close();
	}

	inline T* operator ->() { return mPtr; }

	inline const T* operator ->() const { return mPtr; }

	T* get() { return mPtr; }

private:
	T* mPtr;
};

#endif

