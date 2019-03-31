//
// Copyright (c) 2019 West Coast Code AB. All rights reserved.
//

#include "Mutex.hpp"
#include "../Process/Process.hpp"

Mutex::Mutex(const string& name, bool onHost, OsMutex* mutex)
		: mName(name), mOnHost(onHost), mMutex(mutex) {

}

Mutex::~Mutex() {
	Mutex::Destroy();
}

ESErrorCode Mutex::Lock(uint32_t timeout) {
	if (mMutex == nullptr) {
		return ESERR_MUTEX_ALREADY_DESTROYED;
	}

	return OsMutex::Lock(mMutex, timeout);
}

ESErrorCode Mutex::Unlock() {
	if (mMutex == nullptr) {
		return ESERR_MUTEX_ALREADY_DESTROYED;
	}

	return OsMutex::Unlock(mMutex);
}

void Mutex::ShareWith(Process* process) {

}

void Mutex::Destroy() {
	if (mMutex) {
		OsMutex::Destroy(mMutex);
		mMutex = nullptr;
	}
}

Mutex* Mutex::Create(const string& name) {
	auto const mutex = OsMutex::Create(name);
	if (!mutex) {
		return nullptr;
	}

	return new Mutex(name, true, mutex);
}

Mutex* Mutex::LoadFromProcess(const string& name) {
	return nullptr;
}
