#ifndef _EVERSTORE_STORE_CLIENT_H_
#define _EVERSTORE_STORE_CLIENT_H_

#include "../Shared/everstore.h"

class StoreClient
{
public:
	StoreClient(SOCKET client, IpcHost* host, uint32_t maxBufferSize);

	~StoreClient();

	// Disconnect the client and stop this client thread
	void stop();

	// Start the client thread
	ESErrorCode start();

	// Check to see if this client is running
	inline bool running() const { return mRunning; }

	inline SOCKET handle() const { return mClientSocket; }

	inline mutex_t clientLock() const { return mClientLock; }

private:
	// 
	// Run method
	void run();

	//
	// Handle a request that must be handled by this store client
	ESErrorCode handleRequest(const ESHeader* header, ByteBuffer* memory);

	// Load the next header form the stream - with the associated request data
	ESHeader* loadHeaderFromClient(ByteBuffer* memory);

	// Send the supplied memory block to the client.
	ESErrorCode sendBytesToClient(const ByteBuffer* memory);

private:
	SOCKET mClientSocket;
	IpcHost* mIpcHost;
	uint32_t mMaxBufferSize;
	mutex_t mClientLock;
	atomic_bool mRunning;
	thread mThread;
};

struct StoreClients : list<StoreClient*>
{

	StoreClients();

	~StoreClients();

	void disconnectAllClients();

	void removeClosedClients();
};

#endif
