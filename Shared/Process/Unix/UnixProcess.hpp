//
// Copyright (c) 2019 West Coast Code AB. All rights reserved.
//

#ifndef EVERSTORE_UNIXPROCESS_HPP
#define EVERSTORE_UNIXPROCESS_HPP


#include "../../ESErrorCodes.h"
#include "../ProcessID.h"
#include "../../File/Path.hpp"

struct OsProcess
{
	static constexpr auto InvalidProcess = -1;
	static constexpr auto InvalidPipe = -1;

	pid_t handle;
	int unixSocket;

	static ESErrorCode Start(ProcessID id, const Path& command, const vector<string>& args, int32_t bufferSize,
	                         OsProcess* result);

	static ESErrorCode Destroy(OsProcess* process);

	static ESErrorCode WaitForClosed(OsProcess* process, uint32_t timeout);

	static ESErrorCode ConnectToHost(ProcessID id, int32_t bufferSize, OsProcess* process);

	static int32_t Write(OsProcess* process, const char* bytes, uint32_t size);

	static int32_t Read(OsProcess* process, char* bytes, uint32_t size);

	static bool IsInvalid(const OsProcess* process) { return process == nullptr || process->handle == InvalidProcess; }
};


#endif //EVERSTORE_UNIXPROCESS_HPP
