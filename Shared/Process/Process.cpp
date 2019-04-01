//
// Copyright (c) 2019 West Coast Code AB. All rights reserved.
//

#include "Process.hpp"
#include "../Log/Log.hpp"

Process::Process() {
}

Process::~Process() {
	Process::Destroy();
}

ESErrorCode Process::Destroy() {
	return OsProcess::Destroy(&mProcess);
}

Process* Process::Start(ProcessID id, const Path& command, const vector<string>& args, int32_t bufferSize) {
	auto result = new Process();
	const auto error = OsProcess::Start(id, command, args, bufferSize, &result->mProcess);
	if (isError(error)) {
		delete result;
		Log::Write(Log::Error, "Failed to start '%s': %s (%d)", command.value.c_str(), parseErrorCode(error), error);
		return nullptr;
	}
	return result;
}
