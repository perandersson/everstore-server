#ifndef _EVERSTORE_STORE_SERVER_H_
#define _EVERSTORE_STORE_SERVER_H_

#include "../Shared/everstore.h"
#include "StoreClient.h"
#include "Auth/Authenticator.h"

enum Endian : uint8_t
{
	ES_LITTLE_ENDIAN = 0,
	ES_BIG_ENDIAN
};

struct ServerConfiguration
{
	// Little or big endian
	Endian endian;

	// Protocol version
	char version;

	// Is the user required to authenticate (1 == true)
	char authenticate;
};

class StoreServer
{
public:
	StoreServer(uint16_t port, uint32_t maxConnections, uint32_t maxBufferSize, IpcHost* host,
	            Authenticator* authenticator);

	~StoreServer();

	ESErrorCode listen();

	void close();

	ESErrorCode acceptClient();

	ESErrorCode authenticate(SOCKET socket);

private:
	/**
	 * Gracefully disconnect all clients
	 */
	void disconnectAllClients();

	/**
	 * Garbage collect any running client processes. This is a controlled way to remove already closed clients
	 */
	void garbageCollect();

private:
	const uint16_t mPort;
	const uint32_t mMaxConnections;
	const uint32_t mMaxBufferSize;

	ServerConfiguration mConfiguration;

	SOCKET mServerSocket;
	sockaddr_in mAddr;
	IpcHost* mIpcHost;
	Authenticator* mAuthenticator;
	list<StoreClient*> mClients;
};

#endif
