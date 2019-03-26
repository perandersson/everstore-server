#include "IpcHost.h"
#include "../StringUtils.h"
#include <stdlib.h>
#include <string.h>

IpcHost::IpcHost(const string& rootDir, const string& configFileName, uint32_t maxBufferSize)
		: mRootDir(rootDir), mConfigFileName(configFileName), mMaxBufferSize() {
}

IpcHost::~IpcHost() {
	for (auto client : mProcesses) {
		client->stop();
		delete client;
	}
}

void IpcHost::close() {
	ESHeader header;
	header.type = REQ_SHUTDOWN;

	// Send a shutdown message to all worker processes
	for (auto process : mProcesses) {
		process->child().sendTo(&header);
	}

	// Wait for the process to shutdown and then close all host handles
	waitAndClose();
}

ESErrorCode IpcHost::send(const ESHeader* message) {
	ESErrorCode err = ESERR_NO_ERROR;
	const uint32_t numProcesses = mProcesses.size() + 1;
	for (uint32_t i = 1u; i < numProcesses; ++i) {
		const ChildProcessID id(i);
		auto process = mProcesses[id.asIndex()];
		err = process->child().sendTo(message);
		if (isError(err)) {
			// Log the problem
			error(err);
			log("Trying to restart process: %d", id);

			// Try to restart the worker
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
		// Log the problem
		error(err);
		log("Trying to restart process: %d", childProcessId.value);

		// Try to restart the worker
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
			string("\"") + mConfigFileName + string("\"")
	};
	return process->start(string("everstore_worker"), mRootDir, arguments);
}

ESErrorCode IpcHost::onClientConnected(SOCKET socket, mutex_t lock) {
	ActiveSocket activeSocket;
	activeSocket.socket = socket;
	activeSocket.m = lock;
	mActiveSockets.push_back(activeSocket);

	log("Client %d is now fully connected", socket);
	const uint32_t numProcesses = mProcesses.size() + 1;
	for (uint32_t i = 1u; i < numProcesses; ++i) {
		auto process = mProcesses[i - 1u];
		ESErrorCode err = process_share_socket(process->handle(), socket, lock);
		if (isError(err)) {
			// Log the problem
			error(err);
			log("Trying to restart process: %d", i);

			// Try to restart the worker
			err = tryRestartWorker(ChildProcessID(i));
			if (isError(err)) {
				return err;
			}

			for (auto& activeSocket : mActiveSockets) {
				err = process_share_socket(process->handle(), activeSocket.socket, activeSocket.m);
				if (isError(err)) {
					log("Could not share socket: %d with worker: %d", activeSocket, i);
					return err;
				}
			}
		}
	}

	return ESERR_NO_ERROR;
}

ESErrorCode IpcHost::onClientDisconnected(SOCKET socket) {
	log("Client %d disconnected", socket);
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
			string("\"") + mConfigFileName + string("\"")
	};
	return process->start(string("everstore_worker"), mRootDir, arguments);
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

void IpcHost::waitAndClose() {
	for (auto client : mProcesses) {
		client->waitAndClose();
	}
}

void IpcHost::log(const char* str, ...) {
	va_list arglist;
	va_start(arglist, str);
	char tmp[5096];
	vsprintf(tmp, str, arglist);
	va_end(arglist);

	printf("[INFO]: %s\n", tmp);
}

void IpcHost::error(const char* str, ...) {
	va_list arglist;
	va_start(arglist, str);
	char tmp[5096];
	vsprintf(tmp, str, arglist);
	va_end(arglist);

	printf("[ERROR]: %s\n", tmp);
}

void IpcHost::error(ESErrorCode err) {
	const auto errorStr = parseErrorCode(err);
	printf("[ERROR]: %s\n", errorStr);
}
