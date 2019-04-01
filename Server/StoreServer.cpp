#include "StoreServer.h"

StoreServer::StoreServer(uint16_t port, uint32_t maxConnections, uint32_t maxBufferSize, IpcHost* host,
                         Authenticator* authenticator)
		: mPort(port), mMaxConnections(maxConnections), mMaxBufferSize(maxBufferSize), mServerSocket(nullptr),
		  mIpcHost(host), mAuthenticator(authenticator) {
}

StoreServer::~StoreServer() {
	if (mServerSocket != nullptr) {
		delete mServerSocket;
		mServerSocket = nullptr;
	}
}

ESErrorCode StoreServer::listen() {
	mServerSocket = Socket::CreateBlocking(mMaxBufferSize);
	if (mServerSocket == nullptr)
		return ESERR_SOCKET_CONFIGURE;

	const auto err = mServerSocket->Listen(Port(mPort), mMaxConnections);
	if (isError(err)) {
		delete mServerSocket;
		mServerSocket = nullptr;
		return err;
	}
	return ESERR_NO_ERROR;
}

ESErrorCode StoreServer::acceptClient() {
	auto const newSocket = mServerSocket->AcceptBlocking();
	if (newSocket == nullptr) {
		return ESERR_SOCKET_CONFIGURE;
	}

	// Prepare server configuration
	ServerConfiguration configuration;
	configuration.endian = ES_BIG_ENDIAN;
	configuration.version = VERSION;
	configuration.authenticate = mAuthenticator->required() ? 1 : 0;
	if (Socket::IsLittleEndian()) {
		configuration.endian = ES_LITTLE_ENDIAN;
	}

	const auto sentBytes = newSocket->SendAll((const char*) &configuration, sizeof configuration);
	if (sentBytes != sizeof configuration) {
		Log::Write(Log::Error, "Could not send server properties to client: %d", socket);
		delete newSocket;
		return ESERR_SOCKET_DISCONNECTED;
	}

	ESErrorCode err = authenticate(newSocket);
	if (isError(err)) {
		delete newSocket;
		return err;
	}

	// Garbage collect any client processes that's closed but not removed
	garbageCollect();

	auto const client = new StoreClient(newSocket, mIpcHost, mMaxBufferSize);
	err = mIpcHost->onClientConnected(newSocket, client->clientLock());
	if (isError(err)) {
		delete newSocket;
		return err;
	}

	err = client->start();
	if (err != ESERR_NO_ERROR) {
		delete client;
		delete newSocket;
		return err;
	}

	mClients.push_back(client);
	return ESERR_NO_ERROR;
}

ESErrorCode StoreServer::authenticate(Socket* newSocket) {
	if (!mAuthenticator->required()) {
		return ESERR_NO_ERROR;
	}

	Log::Write(Log::Error, "Client: %d is authenticating", socket);
	ESHeader header;
	auto recvBytes = newSocket->ReceiveAll((char*) &header, sizeof header);
	if (recvBytes != sizeof header || header.type != REQ_AUTHENTICATE) {
		return ESERR_AUTHENTICATION_FAILED;
	}

	Authentication::Request request;
	recvBytes = newSocket->ReceiveAll((char*) &request, sizeof request);
	if (recvBytes != sizeof request) {
		return ESERR_AUTHENTICATION_FAILED;
	}

	// Read the username, but limit the length to 32 bytes
	string username;
	recvBytes = newSocket->ReceiveString<32>(&username, request.usernameLength);
	if (recvBytes != request.usernameLength) {
		return ESERR_AUTHENTICATION_FAILED;
	}

	// Read the password, but limit the length to 32 bytes
	string password;
	recvBytes = newSocket->ReceiveString<32>(&password, request.passwordLength);
	if (recvBytes != request.passwordLength) {
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
	if (mServerSocket != nullptr) {
		mServerSocket->Destroy();
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
