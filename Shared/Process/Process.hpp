//
// Copyright (c) 2019 West Coast Code AB. All rights reserved.
//

#ifndef EVERSTORE_PROCESS_HPP
#define EVERSTORE_PROCESS_HPP

#if defined(_WIN32)

#include "Win32/Win32Process.hpp"

#else

#include "Unix/UnixProcess.hpp"

#endif

class Process
{
public:
	~Process();

	inline int32_t Read(char* buffer, uint32_t size) { return OsProcess::Read(&mProcess, buffer, size); }

	inline int32_t Write(const char* buffer, uint32_t size) { return OsProcess::Write(&mProcess, buffer, size); }

	inline ESErrorCode WaitForClosed(uint32_t timeout = UINT32_MAX) {
		return OsProcess::WaitForClosed(&mProcess, timeout);
	}

	ESErrorCode Destroy();

	static bool IsInvalid(Process* process) { return process == nullptr || OsProcess::IsInvalid(&process->mProcess); }

	/**
	 * Start a new process
	 *
	 * @param id A unique identifier for the process. Useful for inter-process communication
	 * @param command The actual command we want to run
	 * @param args Arguments to the child-process
	 * @param bufferSize How much memory will be allocated for the pipe used for communication
	 * @return A new process if successful; <code>nullptr</code> otherwise
	 */
	static Process* Start(ProcessID id, const Path& command, const vector<string>& args, int32_t bufferSize);

	inline OsProcess* GetHandle() { return &mProcess; }

private:
	Process();

private:
	OsProcess mProcess;
};


#endif //EVERSTORE_PROCESS_HPP
