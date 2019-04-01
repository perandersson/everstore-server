#include "IpcHost.h"
#include "../../Shared/StringUtils.h"
#include <stdlib.h>
#include <string.h>

IpcHost::IpcHost(const string& rootDir, const Path& configPath, uint32_t maxBufferSize)
		: mRootDir(rootDir), mConfigPath(configPath), mMaxBufferSize(maxBufferSize) {
}

void IpcHost::close() {
	ESHeader header;
	header.type = REQ_SHUTDOWN;

	// Send a shutdown message to all worker processes
	for (auto process : mProcesses) {
		process->sendTo(&header);
	}

	// Wait for the process to shutdown and then close all host handles
	for (auto client : mProcesses) {
		delete client;
	}
	mProcesses.clear();
}

ESErrorCode IpcHost::send(const ESHeader* message) {
	ESErrorCode err = ESERR_NO_ERROR;
	const uint32_t numProcesses = mProcesses.size() + 1;
	for (uint32_t i = 1u; i < numProcesses; ++i) {
		const ProcessID id(i);
		auto process = mProcesses[id.AsIndex()];
		err = process->sendTo(message);
		if (isError(err)) {
			Log::Write(Log::Error,
			           "An error occurred while sending data to client %d. Trying to restart the process. Reason: %s (%d)",
			           id, parseErrorCode(err), err);
			process = tryRestartWorker(id);
			if (process) {
				err = process->sendTo(message);
			}
		}
	}
	return err;
}

ESErrorCode IpcHost::send(const ProcessID id, const ByteBuffer* bytes) {
	assert(bytes != nullptr);

	if (!workerExists(id))
		return ESERR_WORKER_UNKNOWN;

	auto process = mProcesses[id.AsIndex()];
	ESErrorCode err = process->sendTo(bytes);
	if (isError(err)) {
		Log::Write(Log::Error,
		           "An error occurred while sending data to client %d. Trying to restart the process. Reason: %s (%d)",
		           id, parseErrorCode(err), err);
		process = tryRestartWorker(id);
		if (process) {
			err = process->sendTo(bytes);
		}
	}
	return err;
}

ESErrorCode IpcHost::addWorker() {
	const auto process = createProcess();
	if (process == nullptr) {
		return ESERR_PROCESS_FAILED_TO_START;
	}
	return ESERR_NO_ERROR;
}

ESErrorCode IpcHost::onClientConnected(Socket* socket, Mutex* lock) {
	ActiveSocket activeSocket;
	activeSocket.socket = socket;
	activeSocket.m = lock;
	mActiveSockets.push_back(activeSocket);

	Log::Write(Log::Info, "Notifying all child-processes that SOCKET(%p) has connected", socket);
	const uint32_t numProcesses = mProcesses.size() + 1;
	for (uint32_t i = 1u; i < numProcesses; ++i) {
		auto process = mProcesses[i - 1u];

		// Start by sharing the socket and the mutex with the current child-process
		ESErrorCode err = ShareSocketAndMutex(socket, lock, process->process());

		// If sharing did not work then try to restart the process and try again
		if (isError(err)) {
			process = tryRestartWorker(ProcessID(i));
			if (!process) {
				return ESERR_PROCESS_CREATE_CHILD;
			}

			// Start by sharing the socket and the mutex with the current child-process
			err = ShareSocketAndMutex(socket, lock, process->process());
			if (isError(err)) {
				return err;
			}
		}
	}

	return ESERR_NO_ERROR;
}

ESErrorCode IpcHost::onClientDisconnected(Socket* socket) {
	Log::Write(Log::Info, "Notifying all child-processes that SOCKET(%p) has disconnected", socket);
	auto it = mActiveSockets.begin();
	const auto end = mActiveSockets.end();
	for (; it != end; ++it) {
		if (it->socket == socket) {
			mActiveSockets.erase(it);
			break;
		}
	}

	// Message header for new connections
	ESHeader header;
	header.type = REQ_CLOSED_CONNECTION;
	header.client = socket->GetHandle();
	sendToAll(&header);
	return ESERR_NO_ERROR;
}

ProcessID IpcHost::workerId(const char* str, uint32_t length) const {
	const char* end = str + length;
	uint32_t hash = 0;
	for (; str != end; ++str)
		hash += *str;
	return ProcessID((uint32_t) (hash % mProcesses.size()) + 1);
}

IpcChild* IpcHost::tryRestartWorker(ProcessID id) {
	if (!workerExists(id))
		return nullptr;

	// Destroy the worker process
	const auto idx = id.AsIndex();
	auto process = mProcesses[idx];
	delete process;
	mProcesses[idx] = nullptr;

	// Create it again
	return createProcess(id);
}

ESErrorCode IpcHost::sendToAll(const ESHeader* header) {
	ESErrorCode error = ESERR_NO_ERROR;
	// Send message to all processes
	for (auto process : mProcesses) {
		error = process->sendTo(header);
		if (isError(error)) {
			return error;
		}
	}
	return error;
}

IpcChild* IpcHost::createProcess() {
	const auto id = ProcessID(mProcesses.size() + 1);
	return createProcess(id);
}

IpcChild* IpcHost::createProcess(ProcessID id) {
	static const Path command = Path::GetWorkingDirectory() + string("/everstore-worker");

	// Start the worker process
	const vector<string> arguments = {
			id.ToString(),
			string("\"") + mConfigPath.value + string("\"")
	};

	auto const process = Process::Start(id, command, arguments, mMaxBufferSize);
	if (process == nullptr) {
		Log::Write(Log::Error, "Failed to start child process");
		return nullptr;
	}
	auto const child = new IpcChild(id, process);
	if (mProcesses.size() < id.AsIndex()) {
		mProcesses[id.AsIndex()] = child;
	} else {
		mProcesses.push_back(child);
	}
	return child;
}

bool IpcHost::workerExists(ProcessID id) {
	const auto worker = id.value - 1;
	auto processes = mProcesses.size();
	return processes > worker;
}

ESErrorCode IpcHost::ShareSocketAndMutex(Socket* socket, Mutex* lock, Process* process) {
	ESHeader header;
	header.type = REQ_NEW_CONNECTION;
	header.client = socket->GetHandle();
	if (process->Write((char*) &header, sizeof(header)) == 0)
		return ESERR_PIPE_WRITE;

	// Start by sharing the socket and the mutex with the current child-process
	ESErrorCode err = socket->ShareWithProcess(process);
	if (isError(err)) {
		Log::Write(Log::Error,
		           "An error occurred when sharing socket with client %d. Trying to restart the process. Reason: %s (%d)",
		           process->GetID(), parseErrorCode(err), err);
		return err;
	} else {
		err = lock->ShareWith(process);
		if (isError(err)) {
			Log::Write(Log::Error,
			           "An error occurred when sharing mutex with client %d. Trying to restart the process. Reason: %s (%d)",
			           process->GetID(), parseErrorCode(err), err);
			return err;
		}
	}

	return ESERR_NO_ERROR;
}
