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
	return OsProcess::Destroy(&mProcess);
}

Process* Process::Start(ProcessID id, const Path& command, const vector<string>& args, int32_t bufferSize) {
	auto result = new Process(id);
	const auto error = OsProcess::Start(id, command, args, bufferSize, &result->mProcess);
	if (isError(error)) {
		delete result;
		Log::Write(Log::Error, "Failed to start '%s': %s (%d)", command.value.c_str(), parseErrorCode(error), error);
		return nullptr;
	}
	return result;
}

Process* Process::ConnectToHost(ProcessID id, int32_t bufferSize) {
	auto result = new Process(id);
	const auto error = OsProcess::ConnectToHost(id, bufferSize, &result->mProcess);
	if (isError(error)) {
		delete result;
		Log::Write(Log::Error, "Failed to connect to host process with ProcessID(%d): %s (%d)", id,
		           parseErrorCode(error), error);
		return nullptr;
	}

	return result;
}
