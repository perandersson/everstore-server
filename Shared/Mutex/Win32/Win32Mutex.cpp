//
// Copyright (c) 2019 West Coast Code AB. All rights reserved.
//

#include "Win32Mutex.hpp"
#include "../../Process/Win32/Win32Process.hpp"

ESErrorCode OsMutex::Create(const string& name, OsMutex* mutex) {
	SECURITY_ATTRIBUTES attributes = {
			sizeof(SECURITY_ATTRIBUTES),
			nullptr,
			TRUE
	};
	mutex->ptr = CreateMutexEx(&attributes, nullptr, 0, MUTEX_ALL_ACCESS);
	return ESERR_NO_ERROR;
}

ESErrorCode OsMutex::Destroy(OsMutex* mutex) {
	if (mutex == nullptr) {
		return ESERR_MUTEX_ALREADY_DESTROYED;
	}

	if (mutex->ptr) {
		CloseHandle(mutex->ptr);
	}
	return ESERR_NO_ERROR;
}

ESErrorCode OsMutex::Lock(OsMutex* mutex, uint32_t timeout) {
	if (!IsInvalid(mutex)) {
		return ESERR_MUTEX_ALREADY_DESTROYED;
	}

	const DWORD ret = WaitForSingleObject(mutex->ptr, 30000L);
	if (ret != WAIT_OBJECT_0) {
		return ESERR_MUTEX_LOCK_FAILED;
	}
	return ESERR_NO_ERROR;
}

ESErrorCode OsMutex::Unlock(OsMutex* mutex) {
	if (!IsInvalid(mutex)) {
		return ESERR_MUTEX_ALREADY_DESTROYED;
	}

	BOOL ret = ReleaseMutex(mutex->ptr);
	if (!ret) {
		return ESERR_MUTEX_UNLOCK_FAILED;
	}
	return ESERR_NO_ERROR;
}

ESErrorCode OsMutex::ShareWith(OsMutex* mutex, OsProcess* process) {
	// Duplicate the mutex so thar we can share it with another process
	HANDLE duplicatedMutex = INVALID_HANDLE_VALUE;
	BOOL result = DuplicateHandle(GetCurrentProcess(), mutex->ptr, process->processInfo.hProcess, &duplicatedMutex, 0,
	                              TRUE, DUPLICATE_SAME_ACCESS);
	if (!result) {
		return ESERR_MUTEX_SHARE;
	}

	// Send the duplicated handle to the child-process
	const auto writtenBytes = OsProcess::Write(process, (char*) &duplicatedMutex, sizeof(HANDLE));
	if (writtenBytes != sizeof(HANDLE)) {
		CloseHandle(duplicatedMutex);
		return ESERR_MUTEX_SHARE;
	}

	return ESERR_NO_ERROR;
}

ESErrorCode OsMutex::LoadFromProcess(OsMutex* mutex, OsProcess* process) {
	auto bytesRead = OsProcess::Read(process, (char*) &mutex->ptr, sizeof(mutex->ptr));
	if (bytesRead != sizeof(mutex->ptr)) {
		return ESERR_PROCESS_ATTACH_SHARED_SOCKET;
	}
	return ESERR_NO_ERROR;
}
