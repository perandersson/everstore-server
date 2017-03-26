#pragma once

template<typename T>
class AutoClosable
{
public:
	AutoClosable(T* ptr) : mPtr(ptr) {}

	~AutoClosable() {
		mPtr->close();
	}

	inline T* operator->() { return mPtr; }

	inline const T* operator->() const { return mPtr; }

	T* get() { return mPtr; }

private:
	T* mPtr;
};
