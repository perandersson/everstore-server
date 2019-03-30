#ifndef _EVERSTORE_IPC_HOST_H_
#define _EVERSTORE_IPC_HOST_H_

#include "../Config.h"
#include "IpcChildProcess.h"
#include "IpcChild.h"
#include "../File/Path.hpp"

struct ActiveSocket
{
	SOCKET socket;
	mutex_t m;
};

class IpcHost
{
public:
	IpcHost(const string& rootDir, const Path& configPath, uint32_t maxBufferSize);

	~IpcHost() = default;

	// Close the host and and all it's clients
	void close();

	// Send a message to all workers without any content
	ESErrorCode send(const ESHeader* header);

	// Send a message to a specific child process
	ESErrorCode send(ChildProcessID childProcessId, const ByteBuffer* bytes);

	// Add a new worker managed by this host
	ESErrorCode addWorker();

	// Method called when a client is connected 
	ESErrorCode onClientConnected(SOCKET socket, mutex_t lock);

	// Method called when a client is disconnected 
	ESErrorCode onClientDisconnected(SOCKET socket);

	// Retrieves a child-process id based on a string with a given length
	ChildProcessID workerId(const char* str, uint32_t length) const;

private:
	// Try to restart the IPC client
	ESErrorCode tryRestartWorker(ChildProcessID id);

	// Create a new process instance
	IpcChildProcess* createProcess();

	// Check to see if the supplied worker exists
	bool workerExists(ChildProcessID id);

	// Send a message over the IPC pipe
	ESErrorCode sendToAll(const ESHeader* header);

	// Send a message over the IPC pipe
	ESErrorCode sendToAll(const ByteBuffer* bytes);

private:
	const string mRootDir;
	const Path mConfigPath;
	const uint32_t mMaxBufferSize;

	vector<IpcChildProcess*> mProcesses;
	vector<ActiveSocket> mActiveSockets;
};

#endif
