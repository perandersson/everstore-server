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
	Log::Write(Log::Debug4, "Mutex(%p) | Locking", this);
	return OsMutex::Lock(&mMutex, timeout);
}

ESErrorCode Mutex::Unlock() {
	Log::Write(Log::Debug4, "Mutex(%p) | Unlocking", this);
	return OsMutex::Unlock(&mMutex);
}

ESErrorCode Mutex::ShareWith(Process* process) {
	Log::Write(Log::Info, "Mutex(%p) | Sharing with Process(%p)", this, process);
	return OsMutex::ShareWith(&mMutex, process->GetHandle());
}

void Mutex::Destroy() {
	Log::Write(Log::Info, "Mutex(%p) | Destroying mutex", this);
	OsMutex::Destroy(&mMutex);
}

Mutex* Mutex::Create(const string& name) {
	auto const result = new Mutex();
	const auto error = OsMutex::Create(name, &result->mMutex);
	if (isError(error)) {
		delete result;
		Log::Write(Log::Error, "Mutex | Failed to create a mutex. %s (%d)", parseErrorCode(error), error);
		return nullptr;
	}

	return result;
}

Mutex* Mutex::LoadFromProcess(Process* process) {
	auto const mutex = new Mutex();
	Log::Write(Log::Info, "Mutex(%p) | Loading from Process(%p)", mutex, process);
	const auto error = OsMutex::LoadFromProcess(&mutex->mMutex, process->GetHandle());
	if (isError(error)) {
		Log::Write(Log::Error, "Mutex(%p) | %s (%d)", mutex, parseErrorCode(error), error);
		delete mutex;
		return nullptr;
	}
	return mutex;
}
