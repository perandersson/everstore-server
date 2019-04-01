#include "StoreClient.h"

StoreClient::StoreClient(Socket* client, IpcHost* host, uint32_t maxBufferSize)
		: mClientSocket(client), mIpcHost(host), mMaxBufferSize(maxBufferSize), mClientLock(nullptr),
		  mRunning(true) {
	const string mutexName = string("everstore_mutex_") + StringUtils::toString((int)mClientSocket->GetHandle());
	mClientLock = Mutex::Create(mutexName);
}

StoreClient::~StoreClient() {
	assert(!running());

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
	Log::Write(Log::Info, "Thread for StoreClient(%p) is starting up", this);

	// Memory for this client
	ByteBuffer memory(mMaxBufferSize);

	//ByteBuffer byteBuffer;
	ESErrorCode err = ESERR_NO_ERROR;
	while (mRunning && !isErrorCodeFatal(err)) {
		// Read message header. If the request header turns out to be invalid, then that means 
		// that:
		// 1. The client is in a bad state. Force a disconnect on the client.
		// 2. The client has disconnected in a nice way and is marked to be stopped.
		ESHeader* header = loadHeaderFromClient(&memory);
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
			err = handleRequest(header, &memory);
		} else {
			err = mIpcHost->send(workerId, &memory);
		}

		// Notify the client if the supplied request failed
		if (IsErrorButNotFatal(err)) {
			Log::Write(Log::Error, "An error occurred while sending data to process %d: %s (%d)", workerId,
			           parseErrorCode(err), err);
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

	Log::Write(Log::Info, "Thread for StoreClient(%p) is shutting down", this);

	if (isError(err)) {
		Log::Write(Log::Error, "Unhandled error occurred for StoreClient(%p): %s (%d)", this, parseErrorCode(err), err);
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
	assert(mClientSocket != nullptr);
	assert(memory != nullptr);

	// Reset the position of the memory
	memory->reset();

	// Get a memory block for the header
	ESHeader* header = memory->allocate<ESHeader>();

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
	assert(mClientSocket != nullptr);
	assert(memory != nullptr);

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
