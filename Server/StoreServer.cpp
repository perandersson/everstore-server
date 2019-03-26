#include "StoreServer.h"

StoreServer::StoreServer(uint16_t port, uint32_t maxConnections, uint32_t maxBufferSize, IpcHost* host, 
	Authenticator* authenticator)
: mPort(port), mMaxConnections(maxConnections), mMaxBufferSize(maxBufferSize), mServerSocket(INVALID_SOCKET), mIpcHost(host), 
mAuthenticator(authenticator) {
	// Prepare socket configuration
	memset(&mAddr, 0, sizeof(sockaddr_in));
	mAddr.sin_family = AF_INET;
	mAddr.sin_addr.s_addr = INADDR_ANY;
	mAddr.sin_port = htons(mPort);

	// Prepare server configuration
	mConfiguration.endian = ES_BIG_ENDIAN;
	mConfiguration.version = VERSION;
	mConfiguration.authenticate = mAuthenticator->required() ? TRUE : FALSE;
	if (is_little_endian()) {
		mConfiguration.endian = ES_LITTLE_ENDIAN;
	}
}

StoreServer::~StoreServer() {
	StoreServer::close();
}

ESErrorCode StoreServer::listen() {
	mServerSocket = socket_create_blocking(mMaxBufferSize);
	if (mServerSocket == INVALID_SOCKET)
		return ESERR_SOCKET_CONFIGURE;

	if (::bind(mServerSocket, (struct sockaddr *) &mAddr, sizeof (mAddr)) == -1) {
		socket_close(mServerSocket);
		mServerSocket = INVALID_SOCKET;
		return ESERR_SOCKET_BIND;
	}

	if (::listen(mServerSocket, mMaxConnections) == -1) {
		socket_close(mServerSocket);
		mServerSocket = INVALID_SOCKET;
		return ESERR_SOCKET_LISTEN;
	}

	return ESERR_NO_ERROR;
}

ESErrorCode StoreServer::acceptClient() {
	SOCKET socket = socket_accept_blocking(mServerSocket, mMaxBufferSize);
	if (socket == INVALID_SOCKET) {
		return ESERR_SOCKET_CONFIGURE;
	}

	if (!socket_sendall(socket, &mConfiguration)) {
		mIpcHost->log("Could not send the server properties to client: %d", socket);
		socket_close(socket);
		return ESERR_SOCKET_DISCONNECTED;
	}

	ESErrorCode err = authenticate(socket);
	if (isError(err)) {
		socket_close(socket);
		return err;
	}

	// Garbage collect any running client processes
	mClients.removeClosedClients();

	StoreClient* client = new StoreClient(socket, mIpcHost, mMaxBufferSize);
	err = mIpcHost->onClientConnected(socket, client->clientLock());
	if (isError(err)) {
		socket_close(socket);
		return err;
	}

	err = client->start();
	if (err != ESERR_NO_ERROR) {
		socket_close(socket);
		return err;
	}

	mClients.push_back(client);
	return ESERR_NO_ERROR;
}

ESErrorCode StoreServer::authenticate(SOCKET socket) {
	if (!mAuthenticator->required()) return ESERR_NO_ERROR;

	mIpcHost->log("Client is authenticating");
	ESHeader header;
	if (socket_recvall(socket, &header) == 0 || header.type != REQ_AUTHENTICATE)
		return ESERR_AUTHENTICATION_FAILED;

	Authentication::Request request;
	if (socket_recvall(socket, &request) == 0)
		return ESERR_AUTHENTICATION_FAILED;

	string username;
	if (socket_recvstring<32>(socket, &username, request.usernameLength) == 0) {
		return ESERR_AUTHENTICATION_FAILED;
	}

	string password;
	if (socket_recvstring<32>(socket, &password, request.passwordLength) == 0) {
		return ESERR_AUTHENTICATION_FAILED;
	}

	if (!mAuthenticator->login(username, password))
		return ESERR_AUTHENTICATION_FAILED;

	mIpcHost->log("Client %d is authenticated as: '%s'", socket, username.c_str());
	return ESERR_NO_ERROR;
}

void StoreServer::close() {
	mClients.disconnectAllClients();
	if (mServerSocket != INVALID_SOCKET) {
		socket_close(mServerSocket);
		mServerSocket = INVALID_SOCKET;
	}
}
