#ifndef _EVERSTORE_IPC_HOST_H_
#define _EVERSTORE_IPC_HOST_H_

#include "../../Shared/Config.h"
#include "../../Shared/Ipc/IpcChild.h"
#include "../../Shared/File/Path.hpp"

struct ActiveSocket
{
	Socket* socket;
	Mutex* m;
};

class IpcHost
{
public:
	IpcHost(const Path& rootDir, const Path& configPath, uint32_t maxBufferSize);

	~IpcHost() = default;

	// Close the host and and all it's clients
	void close();

	// Send a message to all workers without any content
	ESErrorCode send(const ESHeader* header);

	// Send a message to a specific child process
	ESErrorCode send(ProcessID id, const ByteBuffer* bytes);

	// Add a new worker managed by this host
	ESErrorCode addWorker();

	// Method called when a client is connected 
	ESErrorCode onClientConnected(Socket* socket, Mutex* lock);

	// Method called when a client is disconnected 
	ESErrorCode onClientDisconnected(Socket* socket);

	// Retrieves a child-process id based on a string with a given length
	ProcessID workerId(const char* str, uint32_t length) const;

private:
	// Try to restart the IPC client
	IpcChild* tryRestartWorker(ProcessID id);

	// Create a new process instance
	IpcChild* createProcess();

	// Create a new process instance
	IpcChild* createProcess(ProcessID id);

	// Check to see if the supplied worker exists
	bool workerExists(ProcessID id);

	// Send a message over the IPC pipe
	ESErrorCode sendToAll(const ESHeader* header);

	ESErrorCode ShareSocketAndMutex(Socket* socket, Mutex* lock, Process* process);

private:
	const Path mRootDir;
	const Path mConfigPath;
	const uint32_t mMaxBufferSize;

	vector<IpcChild*> mProcesses;
	vector<ActiveSocket> mActiveSockets;
};

#endif
