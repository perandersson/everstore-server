#include "IpcChildProcess.h"
#include "../StringUtils.h"

IpcChildProcess::IpcChildProcess(ChildProcessID id, uint32_t maxBufferSize)
		: mChild(id), mMaxBufferSize(maxBufferSize) {
	process_init(handle());
}

IpcChildProcess::~IpcChildProcess() {
	IpcChildProcess::waitAndClose();
}

void IpcChildProcess::waitAndClose() {
	process_wait(handle());
	stop();
}

void IpcChildProcess::kill() {
	stop();
}

ESErrorCode IpcChildProcess::start(const string& command, const string& currentDirectory,
                                   const vector<string>& arguments) {
	const string name = mChild.id().toString();
	return process_start(name, command, currentDirectory, arguments, mMaxBufferSize, handle());
}

ESErrorCode IpcChildProcess::stop() {
	mChild.close();
	return ESERR_NO_ERROR;
}
