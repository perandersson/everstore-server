#include "IpcChildProcess.h"
#include "../StringUtils.h"

IpcChildProcess::IpcChildProcess(const ChildProcessId id) : mChild(id) {
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

ESErrorCode IpcChildProcess::start(const string& command, const string& currentDirectory, const vector<string>& arguments) {
	const string name = PIPE_NAME_PREFIX + mChild.id().toString();
	return process_start(name, command, currentDirectory, arguments, handle());
}

ESErrorCode IpcChildProcess::stop() {
	process_close(handle());
	mChild.close();
	return ESERR_NO_ERROR;
}

IpcChildProcesses::IpcChildProcesses() {

}

IpcChildProcesses::~IpcChildProcesses() {
	for (auto* client : *this) {
		client->stop();
		delete client;
	}
}

IpcChildProcess* IpcChildProcesses::createProcess() {
	IpcChildProcess* process = new IpcChildProcess(size() + 1);
	push_back(process);
	return process;
}

bool IpcChildProcesses::workerExists(const ChildProcessId id) {
	const auto worker = id.value - 1;
	auto processes = size();
	return processes > worker;
}

void IpcChildProcesses::waitAndClose() {
	for (auto client : *this) {
		client->waitAndClose();
	}
}

IpcChildProcess* IpcChildProcesses::get(const ChildProcessId id) {
	const auto worker = id.value - 1;
	return operator[](worker);
}
