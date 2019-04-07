//
// Copyright (c) 2019 West Coast Code AB. All rights reserved.
//

#include "Process.hpp"
#include "../Log/Log.hpp"

Process::Process(ProcessID id)
		: mId(id) {
}

Process::~Process() {
	Process::Destroy();
}

ESErrorCode Process::Destroy() {
	Log::Write(Log::Info, "Process(%p) | Destroying", this);
	return OsProcess::Destroy(&mProcess);
}

Process* Process::Start(ProcessID id, const Path& command, const vector<string>& args, int32_t bufferSize) {
	auto process = new Process(id);
	Log::Write(Log::Info, "Process(%p) | Starting \"%s\" and associate it with ProcessID(%d)", process,
	           command.value.c_str(), id);
	const auto error = OsProcess::Start(id, command, args, bufferSize, &process->mProcess);
	if (isError(error)) {
		Log::Write(Log::Error, "Process(%p) | %s (%d)", process, parseErrorCode(error), error);
		delete process;
		return nullptr;
	}
	return process;
}

Process* Process::ConnectToHost(ProcessID id, int32_t bufferSize) {
	auto process = new Process(id);
	Log::Write(Log::Info, "Process(%p) | Connecting to the host process as ProcessID(%d)", process, id);
	const auto error = OsProcess::ConnectToHost(id, bufferSize, &process->mProcess);
	if (isError(error)) {
		Log::Write(Log::Error, "Process(%p) | %s (%d)", process, parseErrorCode(error), error);
		delete process;
		return nullptr;
	}

	return process;
}
