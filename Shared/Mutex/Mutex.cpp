//
// Copyright (c) 2019 West Coast Code AB. All rights reserved.
//

#include "Mutex.hpp"
#include "../Process/Process.hpp"
#include "../Log/Log.hpp"

Mutex::Mutex(const string& name, bool onHost)
		: mName(name), mOnHost(onHost) {

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
	auto const result = new Mutex(name, true);
	const auto error = OsMutex::Create(name, &result->mMutex);
	if (isError(error)) {
		delete result;
		Log::Write(Log::Error, "Failed to create a mutex: %s (%d)", parseErrorCode(error), error);
		return nullptr;
	}

	return result;
}

Mutex* Mutex::LoadFromProcess(const string& name, Process* process) {
	auto const result = new Mutex(name, false);
	const auto error = OsMutex::LoadFromProcess(&result->mMutex, process->GetHandle());
	if (isError(error)) {
		delete result;
		Log::Write(Log::Error, "Failed to load a mutex from parent process: %s (%d)", parseErrorCode(error), error);
		return nullptr;
	}
	return result;
}
