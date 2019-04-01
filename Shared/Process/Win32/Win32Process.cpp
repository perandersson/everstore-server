//
// Copyright (c) 2019 West Coast Code AB. All rights reserved.
//

#include "Win32Process.hpp"
#include "../../ESErrorCodes.h"
#include "../../Log/Log.hpp"
#include "../../File/FileUtils.h"

const string gPipeNamePrefix = "\\\\.\\pipe\\everstore";
const uint32_t gDefaultTimeout = 2000;

ESErrorCode Win32CreateNamedPipe(ProcessID id, HANDLE* p, int32_t bufferSize) {
	*p = INVALID_HANDLE_VALUE;

	SECURITY_ATTRIBUTES sec;
	sec.bInheritHandle = TRUE;
	sec.lpSecurityDescriptor = nullptr;
	sec.nLength = sizeof(SECURITY_ATTRIBUTES);

	const auto pipeName = gPipeNamePrefix + id.ToString();
	*p = CreateNamedPipe(pipeName.c_str(), PIPE_ACCESS_DUPLEX | PIPE_TYPE_BYTE | PIPE_READMODE_BYTE, PIPE_WAIT, 1,
	                     bufferSize, bufferSize,
	                     gDefaultTimeout, &sec);

	if (*p == INVALID_HANDLE_VALUE)
		return ESERR_PIPE_CREATE;

	return ESERR_NO_ERROR;
}


ESErrorCode OsProcess::Start(ProcessID id, const Path& command, const vector<string>& args, int32_t bufferSize,
                             OsProcess* process) {
	memset(process, 0, sizeof(OsProcess));

	// Start by creating a pipe that we the sub-process will be connecting to. This must be done before the process
	// is actually started
	HANDLE pipe;
	auto error = Win32CreateNamedPipe(id, &pipe, bufferSize);
	if (isError(error)) {
		Log::Write(Log::Error, "Failed to create named pipe: %s (%d)", parseErrorCode(error), error);
		return error;
	}

	// Start the process itself
	SECURITY_ATTRIBUTES attributes = {
			sizeof(SECURITY_ATTRIBUTES),
			nullptr,
			TRUE
	};
	const auto directory = command.GetDirectory();
	BOOL result = CreateProcess(nullptr, (char*) command.value.c_str(), &attributes, &attributes, TRUE, 0, nullptr,
	                            directory.value.c_str(), &process->startupInfo, &process->processInfo);
	if (!result) {
		CloseHandle(pipe);
		free(process);
		const auto lastError = GetLastError();
		Log::Write(Log::Error, "Failed to start process '%s' (%d)", command.value.c_str(), lastError);
		return ESERR_PROCESS_FAILED_TO_START;
	}
	process->pipe = pipe;
	process->running = true;
	return ESERR_NO_ERROR;
}

ESErrorCode OsProcess::Destroy(OsProcess* process) {
	if (process == nullptr) {
		return ESERR_PROCESS_DESTROYED;
	}

	// Start by closing the pipe
	if (process->pipe != INVALID_HANDLE_VALUE) {
		CloseHandle(process->pipe);
		process->pipe = INVALID_HANDLE_VALUE;
	}

	// Wait and then close the process
	if (process->running) {
		// Wait for 30 seconds until we're closing the processes
		static constexpr auto TimeoutMillis = 30u * 1000u;
		WaitForClosed(process, TimeoutMillis);
		CloseHandle(process->processInfo.hProcess);
		CloseHandle(process->processInfo.hThread);
		// TODO: Do we have to "close handle" the process if it's stopped?
		// TODO: Double-check that this does not turn into a memory-leak
		process->running = false;
	}
	return ESERR_NO_ERROR;
}

ESErrorCode OsProcess::WaitForClosed(OsProcess* process, uint32_t timeout) {
	if (process == nullptr) {
		return ESERR_PROCESS_DESTROYED;
	}

	// Fetch the exit code of the supplied process. Assume that if this fails then the process is shut down
	DWORD exitCode = 0;
	BOOL result = GetExitCodeProcess(process->processInfo.hProcess, &exitCode);
	if (result && exitCode == STILL_ACTIVE) {
		WaitForSingleObject(process->processInfo.hProcess, timeout);
		result = GetExitCodeProcess(process->processInfo.hProcess, &exitCode);
		if (result && exitCode == STILL_ACTIVE) {
			return ESERR_PROCESS_FAILED_TO_CLOSE;
		}
	}

	return ESERR_NO_ERROR;
}

int32_t OsProcess::Write(OsProcess* p, const char* bytes, uint32_t size) {
	if (p == nullptr) {
		return -1;
	}

	int32_t totalBytes = 0;
	BOOL result;
	do {
		DWORD writeBytes = 0;
		result = WriteFile(p->pipe, &bytes[totalBytes], size - totalBytes, &writeBytes, nullptr);
		totalBytes += writeBytes;
	} while (result && totalBytes < (int32_t) size);

	if (totalBytes > 0)
		FlushFileBuffers(p->pipe);

	return totalBytes;
}

int32_t OsProcess::Read(OsProcess* p, char* bytes, uint32_t size) {
	if (p == nullptr) {
		return -1;
	}

	int32_t totalBytes = 0;
	BOOL result;
	do {
		DWORD readBytes = 0;
		result = ReadFile(p->pipe, &bytes[totalBytes], size - totalBytes, &readBytes, nullptr);
		totalBytes += readBytes;
	} while (result && totalBytes < (int32_t) size);
	return totalBytes;
}
