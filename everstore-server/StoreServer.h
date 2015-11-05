#ifndef _EVERSTORE_STORE_SERVER_H_
#define _EVERSTORE_STORE_SERVER_H_

#include <everstore.h>
#include "StoreClient.h"
#include "Authenticator.h"

enum Endian : uint8_t {
	ES_LITTLE_ENDIAN = 0,
	ES_BIG_ENDIAN
};

struct ServerConfiguration {
	Endian endian;			// Little or big endian
	char version;			// Protocol version
	char authenticate;		// Is the user required to authenticate (1 == true)
};

struct StoreServer {

	StoreServer(uint16_t port, uint32_t maxConnections, IpcHost* host, Authenticator* authenticator);

	~StoreServer();

	ESErrorCode listen();

	void close();

	ESErrorCode acceptClient();

	ESErrorCode authenticate(SOCKET socket);

	inline const ServerConfiguration& getConfiguration() const { return mConfiguration; }
	
private:
	uint16_t mPort;
	uint32_t mMaxConnections;

	ServerConfiguration mConfiguration;

	SOCKET mServerSocket;
	sockaddr_in mAddr;
	IpcHost* mIpcHost;
	Authenticator* mAuthenticator;
	StoreClients mClients;
};

#endif
