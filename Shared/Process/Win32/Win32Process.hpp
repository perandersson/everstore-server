//
// Copyright (c) 2019 West Coast Code AB. All rights reserved.
//

#ifndef EVERSTORE_WIN32PROCESS_HPP
#define EVERSTORE_WIN32PROCESS_HPP

#include <windows.h>
#include <cinttypes>
#include "../ProcessID.h"
#include "../../ESErrorCodes.h"
#include "../../File/Path.hpp"

struct OsProcess
{
	PROCESS_INFORMATION processInfo;
	STARTUPINFO startupInfo;
	bool running;
	HANDLE pipe;

	static ESErrorCode Start(ProcessID id, const Path& command, const vector<string>& args, int32_t bufferSize,
	                         OsProcess* result);

	static ESErrorCode Destroy(OsProcess* process);

	static ESErrorCode WaitForClosed(OsProcess* process, uint32_t timeout);

	static int32_t Write(OsProcess* process, const char* bytes, uint32_t size);

	static int32_t Read(OsProcess* process, char* bytes, uint32_t size);

	static bool IsInvalid(OsProcess* process) { return process == nullptr; }
};


#endif //EVERSTORE_WIN32PROCESS_HPP
