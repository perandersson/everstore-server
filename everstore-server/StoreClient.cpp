#include "StoreClient.h"

StoreClient::StoreClient(SOCKET client, IpcHost* host)
		: mClientSocket(client), mClientLock(INVALID_LOCK), mIpcHost(host) {
	mRunning.store(true, memory_order_relaxed);
	const string mutexName = string("everstore_mutex_") + StringUtils::toString(client);
	mClientLock = mutex_create(mutexName);
}

StoreClient::~StoreClient() {
	if (mClientLock != INVALID_LOCK) {
		mutex_destroy(mClientLock);
		mClientLock = INVALID_LOCK;
	}
	if (mClientSocket != INVALID_SOCKET) {
		socket_close(mClientSocket);
		mClientSocket = INVALID_SOCKET;
	}
	mThread.join();
}

ESErrorCode StoreClient::start() {
	if (mClientLock == INVALID_LOCK) {
		return ESERR_CLIENT_MUTEX_FAILED;
	}
	mThread = thread(&StoreClient::run, this);
	return ESERR_NO_ERROR;
}

void StoreClient::run() {
	// Memory for this client
	Bytes memory(DEFAULT_MAX_DATA_SEND_SIZE);

	//ByteBuffer byteBuffer;
	ESErrorCode err = ESERR_NO_ERROR;
	while (running() && !isErrorCodeFatal(err)) {
		// Read message header. If the request header turns out to be invalid, then that means 
		// that the client is in a bad state. Force a disconnect on the client.
		ESHeader* header = loadHeaderFromClient(&memory);
		if (header->type == REQ_INVALID || isInternalRequestType(header->type)) {
			stop();
			return;
		}

		// Prepare the request header and put it into the memory buffer
		const uint32_t workerId = header->workerId;
		const uint32_t requestUID = header->requestUID;
		header->client = mClientSocket;

		// If no worker is specified then let the host figure it out. Otherwise redirect directly to it
		if (isRequestTypeInitiallyForHost(header->type)) {
			err = handleRequest(header, &memory);
		}
		else {
			err = mIpcHost->send(workerId, &memory);
		}

		// Notify the client if the supplied request failed
		if (IsErrorButNotFatal(err)) {
			mIpcHost->error(err);
			err = ESERR_NO_ERROR;

			// Reset memory
			memory.reset();

			// Put the response into the memory block
			const RequestError::Header responseHeader(requestUID, workerId);
			const RequestError::Response response(err);
			memory.put(&responseHeader);
			memory.put(&response);

			// Send the data to the client
			err = sendBytesToClient(&memory);
		}
	}

	if (isError(err)) {
		mIpcHost->error(err);
	}
}

ESErrorCode StoreClient::handleRequest(const ESHeader* header, Bytes* bytes) {
	ESErrorCode err = ESERR_REQUEST_TYPE_UNKNOWN;
	if (header->type == REQ_NEW_TRANSACTION) {
		// Memorize and reset the memory block
		bytes->memorize();
		bytes->reset();

		// Load data about the new transaction
		const auto request = bytes->get<NewTransaction::Request>();
		const auto journalName = IntrusiveBytesString(request->journalStringLength, bytes);

		// Figure out the worker for the requested journal
		const auto workerId = mIpcHost->workerId(journalName.str, journalName.length);

		// Restore the memorized position
		bytes->restore();

		// Send the request to the supplied worker
		err = mIpcHost->send(workerId, bytes);
	}

	return err;
}

ESHeader* StoreClient::loadHeaderFromClient(Bytes* memory) {
	assert(mClientSocket != INVALID_SOCKET);
	assert(memory != nullptr);

	// Reset the position of the memory
	memory->reset();

	// Get a memory block for the header
	ESHeader* header = memory->get<ESHeader>();

	// Read the header from client socket stream
	if (socket_recvall(mClientSocket, (char*)header, sizeof(ESHeader)) != sizeof(ESHeader))
		return &INVALID_HEADER;

	// Validate request
	if (header->size > DEFAULT_MAX_DATA_SEND_SIZE) return &INVALID_HEADER;

	// Load the request body
	if (header->size > 0) {
		if (socket_recvall(mClientSocket, memory->get(header->size), header->size) != header->size) {
			return &INVALID_HEADER;
		}
	}

	// Do not allow multipart requests yet!
	if (header->properties != ESPROP_NONE) return &INVALID_HEADER;
	return header;
}

ESErrorCode StoreClient::sendBytesToClient(const Bytes* memory) {
	assert(mClientSocket != INVALID_SOCKET);
	assert(memory != nullptr);

	// The current offset indicates how much memory we've written to the memory
	const uint32_t size = memory->offset();

	mutex_lock(mClientLock);
	const uint32_t recv = socket_sendall(mClientSocket, memory->ptr(), size);
	mutex_unlock(mClientLock);

	// Verify that we've sent all the data to the client
	if (recv != size) return ESERR_SOCKET_SEND;
	return ESERR_NO_ERROR;
}

void StoreClient::stop() {
	socket_close(mClientSocket);
	mIpcHost->onClientDisconnected(mClientSocket);
	mClientSocket = INVALID_SOCKET;
	mRunning.store(false, memory_order_relaxed);
}

StoreClients::StoreClients() {

}

StoreClients::~StoreClients() {

}

void StoreClients::disconnectAllClients() {
	for (auto client : *this) {
		client->stop();
		delete client;
	}
	clear();
}

void StoreClients::removeClosedClients() {
	// Find all clients that we can remove
	list<list<StoreClient*>::iterator> removables;
	auto it = begin();
	const auto e = end();
	for (; it != e; ++it) {
		auto client = *it;
		if (!client->running()) {
			removables.push_back(it);
		}
	}

	// Delete any removable clients
	for (auto removable : removables) {
		delete *removable;
		erase(removable);
	}
}
