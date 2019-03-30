#include "IpcHost.h"
#include "../StringUtils.h"
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
		process->child().sendTo(&header);
	}

	// Wait for the process to shutdown and then close all host handles
	for (auto client : mProcesses) {
		client->waitAndClose();
		delete client;
	}
	mProcesses.clear();
}

ESErrorCode IpcHost::send(const ESHeader* message) {
	ESErrorCode err = ESERR_NO_ERROR;
	const uint32_t numProcesses = mProcesses.size() + 1;
	for (uint32_t i = 1u; i < numProcesses; ++i) {
		const ChildProcessID id(i);
		auto process = mProcesses[id.asIndex()];
		err = process->child().sendTo(message);
		if (isError(err)) {
			Log::Write(Log::Error,
			           "An error occurred while sending data to client %d. Trying to restart the process. Reason: %s (%d)",
			           id, parseErrorCode(err), err);
			err = tryRestartWorker(id);
			if (!isError(err)) {
				err = process->child().sendTo(message);
			}
		}
	}
	return err;
}

ESErrorCode IpcHost::send(const ChildProcessID childProcessId, const ByteBuffer* bytes) {
	assert(bytes != nullptr);

	if (!workerExists(childProcessId))
		return ESERR_WORKER_UNKNOWN;

	auto process = mProcesses[childProcessId.asIndex()];
	ESErrorCode err = process->child().sendTo(bytes);
	if (isError(err)) {
		Log::Write(Log::Error,
		           "An error occurred while sending data to client %d. Trying to restart the process. Reason: %s (%d)",
		           childProcessId, parseErrorCode(err), err);
		err = tryRestartWorker(childProcessId);
		if (!isError(err)) {
			err = process->child().sendTo(bytes);
		}
	}
	return err;
}

ESErrorCode IpcHost::addWorker() {
	const auto process = createProcess();

	// Start the worker process
	const vector<string> arguments = {
			process->id().toString(),
			string("\"") + mConfigPath.value + string("\"")
	};
	return process->start(string("everstore-worker"), mRootDir, arguments);
}

ESErrorCode IpcHost::onClientConnected(SOCKET socket, mutex_t lock) {
	ActiveSocket activeSocket;
	activeSocket.socket = socket;
	activeSocket.m = lock;
	mActiveSockets.push_back(activeSocket);

	Log::Write(Log::Info, "Notifying all child-processes that SOCKET(%p) has connected", socket);
	const uint32_t numProcesses = mProcesses.size() + 1;
	for (uint32_t i = 1u; i < numProcesses; ++i) {
		auto process = mProcesses[i - 1u];
		ESErrorCode err = process_share_socket(process->handle(), socket, lock);
		if (isError(err)) {
			Log::Write(Log::Error,
			           "An error occurred while sending data to client %d. Trying to restart the process. Reason: %s (%d)",
			           i, parseErrorCode(err), err);
			err = tryRestartWorker(ChildProcessID(i));
			if (isError(err)) {
				return err;
			}

			for (auto& active : mActiveSockets) {
				err = process_share_socket(process->handle(), active.socket, active.m);
				if (isError(err)) {
					Log::Write(Log::Error,
					           "Could not share socket: %d with worker: %d: %s (%d)",
					           active, i, parseErrorCode(err), err);
					return err;
				}
			}
		}
	}

	return ESERR_NO_ERROR;
}

ESErrorCode IpcHost::onClientDisconnected(SOCKET socket) {
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
	header.client = socket;
	sendToAll(&header);
	return ESERR_NO_ERROR;
}

ChildProcessID IpcHost::workerId(const char* str, uint32_t length) const {
	const char* end = str + length;
	uint32_t hash = 0;
	for (; str != end; ++str)
		hash += *str;
	return ChildProcessID((uint32_t) (hash % mProcesses.size()) + 1);
}

ESErrorCode IpcHost::tryRestartWorker(ChildProcessID id) {
	if (!workerExists(id))
		return ESERR_WORKER_UNKNOWN;

	// Kill the worker
	auto process = mProcesses[id.asIndex()];
	process->kill();

	// Start the worker process
	const vector<string> arguments = {
			process->id().toString(),
			string("\"") + mConfigPath.value + string("\"")
	};
	return process->start(string("everstore-worker"), mRootDir, arguments);
}

ESErrorCode IpcHost::sendToAll(const ESHeader* header) {
	ESErrorCode error = ESERR_NO_ERROR;
	// Send message to all processes
	for (auto process : mProcesses) {
		error = process->child().sendTo(header);
		if (isError(error)) {
			return error;
		}
	}
	return error;
}

ESErrorCode IpcHost::sendToAll(const ByteBuffer* bytes) {
	ESErrorCode error = ESERR_NO_ERROR;
	// Send message to all processes
	for (auto process : mProcesses) {
		error = process->child().sendTo(bytes);
		if (isError(error)) {
			return error;
		}
	}
	return error;
}

IpcChildProcess* IpcHost::createProcess() {
	IpcChildProcess* process = new IpcChildProcess(ChildProcessID(mProcesses.size() + 1), mMaxBufferSize);
	mProcesses.push_back(process);
	return process;
}

bool IpcHost::workerExists(ChildProcessID id) {
	const auto worker = id.value - 1;
	auto processes = mProcesses.size();
	return processes > worker;
}
