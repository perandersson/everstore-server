//
// Copyright (c) 2019 West Coast Code AB. All rights reserved.
//

#include "Mutex.hpp"
#include "../Process/Process.hpp"
#include "../Log/Log.hpp"

Mutex::Mutex() {

}

Mutex::~Mutex() {
	Mutex::Destroy();
}

ESErrorCode Mutex::Lock(uint32_t timeout) {
	return OsMutex::Lock(&mMutex, timeout);
}

ESErrorCode Mutex::Unlock() {
	return OsMutex::Unlock(&mMutex);
}

ESErrorCode Mutex::ShareWith(Process* process) {
	return OsMutex::ShareWith(&mMutex, process->GetHandle());
}

void Mutex::Destroy() {
	OsMutex::Destroy(&mMutex);
}

Mutex* Mutex::Create(const string& name) {
	auto const result = new Mutex();
	const auto error = OsMutex::Create(name, &result->mMutex);
	if (isError(error)) {
		delete result;
		Log::Write(Log::Error, "Failed to create a mutex: %s (%d)", parseErrorCode(error), error);
		return nullptr;
	}

	return result;
}

Mutex* Mutex::LoadFromProcess(Process* process) {
	OsMutex mutex;
	const auto error = OsMutex::LoadFromProcess(&mutex, process->GetHandle());
	if (isError(error)) {
		Log::Write(Log::Error, "Failed to load a mutex from parent process: %s (%d)", parseErrorCode(error), error);
		return nullptr;
	}
	auto const result = new Mutex();
	result->mMutex = mutex;
	return result;
}
