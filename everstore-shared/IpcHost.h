#ifndef _EVERSTORE_IPC_HOST_H_
#define _EVERSTORE_IPC_HOST_H_

#include "Properties.h"
#include "IpcChildProcess.h"
#include "IpcChild.h"

struct ActiveSocket {
	SOCKET socket;
	mutex_t m;
};

struct IpcHost {

	IpcHost(const string& rootDir, const string& configFileName, uint32_t numWorkers);

	~IpcHost();

	// Close the host and and all it's clients
	void close();

	// Send a message to all workers without any content
	ESErrorCode send(const ESHeader* header);

	// Send a message to a specific child process
	ESErrorCode send(const ChildProcessId childProcessId, const Bytes* bytes);

	// Add a new worker managed by this host
	ESErrorCode addWorker();

	// Method called when a client is connected 
	ESErrorCode onClientConnected(SOCKET socket, mutex_t lock);

	// Method called when a client is disconnected 
	ESErrorCode onClientDisconnected(SOCKET socket);

	// Logging
	void log(const char* str, ...);

	// Logging
	void error(const char* str, ...);

	// Log an error message
	void error(ESErrorCode err);

	// Retrieves a child-process id based on a string with a given length
	const ChildProcessId workerId(const char* str, uint32_t length) const;

private:
	// Try to restart the IPC client
	ESErrorCode tryRestartWorker(const ChildProcessId id);

private:
	const string mRootDir;
	const string mConfigFileName;
	uint32_t mNumWorkers;

	IpcChildProcesses mProcesses;
	vector<ActiveSocket> mActiveSockets;
};

#endif
