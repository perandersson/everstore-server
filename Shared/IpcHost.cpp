#include "IpcHost.h"
#include "StringUtils.h"
#include <stdlib.h>
#include <string.h>

IpcHost::IpcHost(const string& rootDir, const string& configFileName, uint32_t numWorkers) 
: mRootDir(rootDir), mConfigFileName(configFileName), mNumWorkers(numWorkers) {
}

IpcHost::~IpcHost() {
}

void IpcHost::close() {
	ESHeader header;
	header.type = REQ_SHUTDOWN;

	// Send a shutdown message to all worker processes
	for (auto process : mProcesses) {
		process->child().sendTo(&header);
	}

	// Wait for the process to shutdown and then close all host handles
	mProcesses.waitAndClose();
}

ESErrorCode IpcHost::send(const ESHeader* message) {
	ESErrorCode err = ESERR_NO_ERROR;
	const uint32_t numProcesses = mProcesses.size() + 1;
	for (uint32_t i = 1; i < numProcesses; ++i) {
		auto process = mProcesses.get(i);
		err = process->child().sendTo(message);
		if (isError(err)) {
			// Log the problem
			error(err);
			log("Trying to restart process: %d", i);

			// Try to restart the worker
			err = tryRestartWorker(i);
			if (!isError(err)) {
				err = process->child().sendTo(message);
			}
		}
	}
	return err;
}

ESErrorCode IpcHost::send(const ChildProcessId childProcessId, const Bytes* bytes) {
	assert(bytes != nullptr);

	if (!mProcesses.workerExists(childProcessId))
		return ESERR_WORKER_UNKNOWN;

	auto process = mProcesses.get(childProcessId);
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
	const auto process = mProcesses.createProcess();

	// Start the worker process
	const vector<string> arguments = { 
		process->id().toString(), 
		string("\"") + mConfigFileName + string("\"") 
	};
	return process->start(string("everstore-worker"), mRootDir, arguments);
}

ESErrorCode IpcHost::onClientConnected(SOCKET socket, mutex_t lock) {
	ActiveSocket activeSocket;
	activeSocket.socket = socket;
	activeSocket.m = lock;
	mActiveSockets.push_back(activeSocket);

	log("Client %d is now fully connected", socket);
	const uint32_t numProcesses = mProcesses.size() + 1;
	for (uint32_t i = 1; i < numProcesses; ++i) {
		auto process = mProcesses.get(i);
		ESErrorCode err = process_share_socket(process->handle(), socket, lock);
		if (isError(err)) {
			// Log the problem
			error(err);
			log("Trying to restart process: %d", i);

			// Try to restart the worker
			err = tryRestartWorker(i);
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
	for(; it != end; ++it) {
		if (it->socket == socket) {
			mActiveSockets.erase(it);
			break;
		}
	}

	// Message header for new connections
	ESHeader header;
	header.type = REQ_CLOSED_CONNECTION;
	header.client = socket;

	// Send message
	for (auto process : mProcesses) {
		const ESErrorCode err = process->child().sendTo(&header);
		if (isError(err)) {
			return err;
		}
	}
	return ESERR_NO_ERROR;
}

ESErrorCode IpcHost::tryRestartWorker(const ChildProcessId id) {
	if (!mProcesses.workerExists(id))
		return ESERR_WORKER_UNKNOWN;

	// Kill the worker
	auto process = mProcesses.get(id);
	process->kill();

	// Start the worker process
	const vector<string> arguments = {
		process->id().toString(),
		string("\"") + mConfigFileName + string("\"")
	};
	return process->start(string("everstore-worker"), mRootDir, arguments);
}

const ChildProcessId IpcHost::workerId(const char* str, uint32_t length) const {
	const char* end = str + length;
	uint32_t hash = 0;
	for (; str != end; ++str)
		hash += *str;
	return ChildProcessId((uint32_t)(hash % mNumWorkers) + 1);
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
