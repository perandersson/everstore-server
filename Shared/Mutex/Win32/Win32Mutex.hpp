//
// Copyright (c) 2019 West Coast Code AB. All rights reserved.
//

#ifndef EVERSTORE_WIN32MUTEX_HPP
#define EVERSTORE_WIN32MUTEX_HPP

#include <windows.h>
#include <string>
#include "../../ESErrorCodes.h"

using std::string;

struct OsProcess;

struct OsMutex
{
	HANDLE ptr;

	static ESErrorCode Create(const string& name, OsMutex* mutex);

	static ESErrorCode Destroy(OsMutex* mutex);

	static ESErrorCode Lock(OsMutex* mutex, uint32_t timeout);

	static ESErrorCode Unlock(OsMutex* mutex);

	static ESErrorCode ShareWith(OsMutex* mutex, OsProcess* process);

	static ESErrorCode LoadFromProcess(OsMutex* mutex, OsProcess* process);

	inline static bool IsInvalid(const OsMutex* mutex) { return mutex == nullptr || mutex->ptr == INVALID_HANDLE_VALUE; }
};

#endif //EVERSTORE_WIN32MUTEX_HPP
