#include "StoreServer.h"

StoreServer::StoreServer(uint16_t port, uint32_t maxConnections, uint32_t maxBufferSize, IpcHost* host,
                         Authenticator* authenticator)
		: mPort(port), mMaxConnections(maxConnections), mMaxBufferSize(maxBufferSize), mServerSocket(INVALID_SOCKET),
		  mIpcHost(host), mAuthenticator(authenticator) {

}

ESErrorCode StoreServer::listen() {
	mServerSocket = socket_create_blocking(mMaxBufferSize);
	if (mServerSocket == INVALID_SOCKET)
		return ESERR_SOCKET_CONFIGURE;

	// Prepare socket configuration
	sockaddr_in addr;
	memset(&addr, 0, sizeof(sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(mPort);
	if (::bind(mServerSocket, (struct sockaddr*) &addr, sizeof(addr)) == -1) {
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

	// Prepare server configuration
	ServerConfiguration configuration;
	configuration.endian = ES_BIG_ENDIAN;
	configuration.version = VERSION;
	configuration.authenticate = mAuthenticator->required() ? 1 : 0;
	if (is_little_endian()) {
		configuration.endian = ES_LITTLE_ENDIAN;
	}

	if (!socket_sendall(socket, &configuration)) {
		Log::Write(Log::Error, "Could not send server properties to client: %d", socket);
		socket_close(socket);
		return ESERR_SOCKET_DISCONNECTED;
	}

	ESErrorCode err = authenticate(socket);
	if (isError(err)) {
		socket_close(socket);
		return err;
	}

	// Garbage collect any client processes that's closed but not removed
	garbageCollect();

	auto const client = new StoreClient(socket, mIpcHost, mMaxBufferSize);
	err = mIpcHost->onClientConnected(socket, client->clientLock());
	if (isError(err)) {
		socket_close(socket);
		return err;
	}

	err = client->start();
	if (err != ESERR_NO_ERROR) {
		delete client;
		socket_close(socket);
		return err;
	}

	mClients.push_back(client);
	return ESERR_NO_ERROR;
}

ESErrorCode StoreServer::authenticate(SOCKET socket) {
	if (!mAuthenticator->required()) return ESERR_NO_ERROR;

	Log::Write(Log::Error, "Client: %d is authenticating", socket);
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

	Log::Write(Log::Error, "Client: %d has authenticated as %s", socket, username.c_str());
	return ESERR_NO_ERROR;
}

void StoreServer::close() {
	// Close all running connections
	disconnectAllClients();

	// Close the socket used when listening for incoming requests
	if (mServerSocket != INVALID_SOCKET) {
		socket_close(mServerSocket);
		mServerSocket = INVALID_SOCKET;
	}
}

void StoreServer::disconnectAllClients() {
	for (auto client : mClients) {
		client->stop();
		delete client;
	}
	mClients.clear();
}

void StoreServer::garbageCollect() {
	// Find all clients that we can remove
	list<list<StoreClient*>::iterator> removables;
	auto it = mClients.begin();
	const auto e = mClients.end();
	for (; it != e; ++it) {
		auto client = *it;
		if (!client->running()) {
			mIpcHost->onClientDisconnected(client->handle());
			removables.push_back(it);
		}
	}

	// Delete any removable clients
	for (auto removable : removables) {
		delete *removable;
		mClients.erase(removable);
	}
}
