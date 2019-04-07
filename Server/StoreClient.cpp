#include "StoreClient.h"

StoreClient::StoreClient(Socket* client, IpcHost* host, uint32_t maxBufferSize)
		: mClientSocket(client), mIpcHost(host), mMaxBufferSize(maxBufferSize), mClientLock(nullptr),
		  mRunning(true) {
	const string mutexName = string("everstore_mutex_") + StringUtils::toString((int) mClientSocket->GetHandle());
	mClientLock = Mutex::Create(mutexName);
}

StoreClient::~StoreClient() {
	if (mClientSocket != nullptr) {
		delete mClientSocket;
		mClientSocket = nullptr;
	}

	if (mClientLock != nullptr) {
		delete mClientLock;
		mClientLock = nullptr;
	}

	mThread.join();
}

ESErrorCode StoreClient::start() {
	if (mClientLock == nullptr) {
		return ESERR_CLIENT_MUTEX_FAILED;
	}
	mThread = thread(&StoreClient::run, this);
	return ESERR_NO_ERROR;
}

void StoreClient::run() {
	Log::Write(Log::Info, "StoreClient(%p) | Starting up thread", this);

	// Client loop. Keep running as long as the client is alive and no fatal errors has occurred.
	ByteBuffer memory(mMaxBufferSize);
	ESErrorCode err = ESERR_NO_ERROR;
	while (mRunning && !isErrorCodeFatal(err)) {
		// Read message header. If the request header turns out to be invalid, then that means 
		// that:
		// 1. The client is in a bad state. Force a disconnect on the client.
		// 2. The client has disconnected in a nice way and is marked to be stopped.
		ESHeader* header = loadHeaderFromClient(&memory);
		Log::Write(Log::Debug2, "StoreClient(%p) | Received message %s (%d)", this,
		           parseRequestType(header->type), header->type, header->client);
		if (header->type == REQ_INVALID || isInternalRequestType(header->type)) {
			mRunning = false;
			break;
		}

		// Prepare the request header and put it into the memory buffer
		const auto workerId = header->workerId;
		const auto requestUID = header->requestUID;
		header->client = mClientSocket->GetHandle();

		// If no worker is specified then let the host figure it out. Otherwise redirect directly to it
		if (isRequestTypeInitiallyForHost(header->type)) {
			Log::Write(Log::Debug, "StoreClient(%p) | Handling internal message", this);
			err = handleRequest(header, &memory);
		} else {
			Log::Write(Log::Debug, "StoreClient(%p) | Sending request to ProcessID(%d)", this, workerId);
			err = mIpcHost->send(workerId, &memory);
		}

		// Notify the client if the supplied request failed
		if (IsErrorButNotFatal(err)) {
			Log::Write(Log::Error, "StoreClient(%p) | Error occurred: %s (%d)", this, parseErrorCode(err), err);
			err = ESERR_NO_ERROR;

			// Reset memory
			memory.reset();

			// Put the response into the memory block
			const RequestError::Header responseHeader(requestUID, workerId);
			const RequestError::Response response(err);
			memory.write(&responseHeader);
			memory.write(&response);

			// Send the data to the client
			err = sendBytesToClient(&memory);
		}
	}

	Log::Write(Log::Info, "StoreClient(%p) | Shutting down thread", this);
	if (isError(err)) {
		Log::Write(Log::Error, "StoreClient(%p) | Unhandled error occurred: %s (%d)", this, parseErrorCode(err), err);
	}
}

ESErrorCode StoreClient::handleRequest(const ESHeader* header, ByteBuffer* bytes) {
	ESErrorCode err = ESERR_REQUEST_TYPE_UNKNOWN;
	if (header->type == REQ_NEW_TRANSACTION) {
		// Memorize and reset the memory block
		bytes->memorize();
		bytes->reset();

		// Load data about the new transaction
		const auto request = bytes->allocate<NewTransaction::Request>();
		const auto journalName = MutableString(request->journalStringLength, bytes);

		// Figure out the worker for the requested journal
		const auto workerId = mIpcHost->workerId(journalName.str, journalName.length);

		// Restore the memorized position
		bytes->restore();

		// Send the request to the supplied worker
		err = mIpcHost->send(workerId, bytes);
	} else if (header->type == REQ_JOURNAL_EXISTS) {
		// Memorize and reset the memory block
		bytes->memorize();
		bytes->reset();

		// Load data about the new transaction
		const auto request = bytes->allocate<JournalExists::Request>();
		const auto journalName = MutableString(request->journalStringLength, bytes);

		// Figure out the worker for the requested journal
		const auto workerId = mIpcHost->workerId(journalName.str, journalName.length);

		// Restore the memorized position
		bytes->restore();

		// Send the request to the supplied worker
		err = mIpcHost->send(workerId, bytes);
	}

	return err;
}

ESHeader* StoreClient::loadHeaderFromClient(ByteBuffer* memory) {
	// Reset the position of the memory
	memory->reset();

	// Get a memory block for the header
	auto const header = memory->allocate<ESHeader>();

	// Read the header from client socket stream
	if (mClientSocket->ReceiveAll((char*) header, sizeof(ESHeader)) != sizeof(ESHeader))
		return &INVALID_HEADER;

	// Validate request
	if (header->size > (int32_t) mMaxBufferSize) {
		return &INVALID_HEADER;
	}

	// Load the request body
	if (header->size > 0) {
		if (mClientSocket->ReceiveAll(memory->allocate(header->size), header->size) != header->size) {
			return &INVALID_HEADER;
		}
	}

	// Do not allow multipart requests yet!
	if (header->properties != ESPROP_NONE) {
		return &INVALID_HEADER;
	}
	return header;
}

ESErrorCode StoreClient::sendBytesToClient(const ByteBuffer* memory) {
	// The current offset indicates how much memory we've written to the memory
	const uint32_t size = memory->offset();

	// It's important to lock before doing this because any of the sub-processes might be using this socket.
	// Note sub-processes is ever only sending data over the socket and is never reading data.
	mClientLock->Lock();
	const uint32_t sentBytes = mClientSocket->SendAll(memory->ptr(), size);
	mClientLock->Unlock();

	// Verify that we've sent all the data to the client
	if (sentBytes != size) {
		return ESERR_SOCKET_SEND;
	}
	return ESERR_NO_ERROR;
}

void StoreClient::stop() {
	mRunning = false;
}
