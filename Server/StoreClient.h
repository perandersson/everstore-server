#ifndef _EVERSTORE_STORE_CLIENT_H_
#define _EVERSTORE_STORE_CLIENT_H_

#include "../Shared/everstore.h"
#include "Ipc/IpcHost.h"

class StoreClient
{
public:
	StoreClient(Socket* client, IpcHost* host, uint32_t maxBufferSize);

	~StoreClient();

	// Disconnect the client and stop this client thread
	void stop();

	// Start the client thread
	ESErrorCode start();

	/**
	 * Is the client running or not?
	 *
	 * @return <code>true</code> if this client is running
	 */
	inline bool running() const { return mRunning; }

	inline Socket* handle() const { return mClientSocket; }

	inline Mutex* clientLock() const { return mClientLock; }

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
	Socket* mClientSocket;
	IpcHost* mIpcHost;
	uint32_t mMaxBufferSize;
	Mutex* mClientLock;
	atomic_bool mRunning;
	thread mThread;
};

#endif
