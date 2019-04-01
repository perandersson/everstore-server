#include "Worker.h"
#include "../Shared/StringUtils.h"
#include "../Shared/Database/Journal.h"
#include "../Shared/Database/Transaction.h"
#include "../Shared/Bits.hpp"
#include "../Shared/File/Path.hpp"
#include "../Shared/Socket/Socket.hpp"

Worker::Worker(ChildProcessID childProcessId, const Config& config)
		: mIpcChild(childProcessId), mJournals(childProcessId, config.maxJournalLifeTime),
		  mNextTransactionTypeBit(1),
		  mConfig(config) {
}

Worker::~Worker() {

}

ESErrorCode Worker::start() {
	// Make sure that the journals are consistent
	if (!performConsistencyCheck()) {
		return ESERR_STORE_CONSISTENCY_CHECK_FAILED;
	}

	// Initialize worker
	ESErrorCode err = initialize();
	if (isError(err)) {
		return err;
	}

	// Memory for this worker
	ByteBuffer memory(mConfig.maxBufferSize);
	while (mRunning.load() && !isErrorCodeFatal(err)) {
		err = ESERR_NO_ERROR;

		// Load the next data block to be processed from the host
		ESHeader* header = loadHeaderFromHost(&memory);
		if (!isRequestTypeValid(header->type)) {
			err = ESERR_PIPE_READ;
			continue;
		}

		// Get the request type
		const ESRequestType type = header->type;

		// Ignore message if an error occurred while parsing it
		if (!isError(err)) {
			if (isInternalRequestType(type)) {
				err = handleHostMessage(header);
			} else {
				const AttachedConnection* attachedSocket = mAttachedSockets.get(header->client);
				if (attachedSocket->socket != INVALID_SOCKET) {
					err = handleMessage(header, attachedSocket, &memory);
				} else {
					err = ESERR_SOCKET_NOT_ATTACHED;
				}

				// If the error is not fatal then log it, send it to the client and 
				// continue working on the next message
				if (IsErrorButNotFatal(err)) {
					Log::Write(Log::Error, "%s (%d)", parseErrorCode(err), err);

					// Send the error to client
					if (err != ESERR_SOCKET_NOT_ATTACHED) {
						// Reset memory
						memory.reset();

						// Create response header
						const RequestError::Header responseHeader(type, id());
						const RequestError::Response response(err);
						memory.write(&responseHeader);
						memory.write(&response);

						// Send the data to the client
						err = sendBytesToClient(attachedSocket, &memory);
					}
				}
			}
		}
	}

	release();
	return err;
}

void Worker::stop() {
	mRunning = false;
}

ESErrorCode Worker::sendBytesToClient(const AttachedConnection* connection, const ByteBuffer* memory) {
	assert(connection != nullptr);
	assert(memory != nullptr);

	// The current offset indicates how much memory we've written to the memory
	const uint32_t size = memory->offset();

	mutex_lock(connection->lock);
	const uint32_t recv = socket_sendall(connection->socket, memory->ptr(), size);
	mutex_unlock(connection->lock);

	// Verify that we've sent all the data to the client
	if (recv != size) return ESERR_SOCKET_SEND;
	return ESERR_NO_ERROR;
}

ESErrorCode Worker::initialize() {
	// Initialize sockets
	if (!Socket::Initialize()) {
		return ESERR_SOCKET_INIT;
	}

	Log::Write(Log::Info, "Opening pipe to host");
	ESErrorCode err = mIpcChild.connectToHost();
	if (isError(err)) {
		return err;
	}

	const auto path = mConfig.rootDir + Path::StrPathDelim + mConfig.journalDir;
	Log::Write(Log::Info, "Changing working directory to: %s", path.c_str());
	if (!FileUtils::setCurrentDirectory(path)) {
		return ESERR_FILESYSTEM_CHANGEPATH;
	}

	Log::Write(Log::Info, "Preparing built-in transaction types");
	vector<string> types;
	types.push_back(Bits::BuiltIn::NewJournalKey);
	auto mask = transactionTypes(types);
	assert(mask == Bits::BuiltIn::NewJournalBit);

	Log::Write(Log::Info, "Initialization complete");
	mRunning = true;
	return ESERR_NO_ERROR;
}

void Worker::release() {
	mIpcChild.close();
	mAttachedSockets.clear();
	Socket::Shutdown();
}

bool Worker::performConsistencyCheck() {
	const string lockSufix(string(".log.") + id().toString() + string(".lock"));
	auto files = FileUtils::findFilesEndingWith(mConfig.journalDir, lockSufix);
	for (auto& file : files) {
		const Path journalFile = Path(file.substr(0, file.length() - lockSufix.length()));
		Log::Write(Log::Info, "Validating consistency for journal: %s", journalFile.value.c_str());
		Journal j(journalFile, id());
		if (!j.performConsistencyCheck()) {
			Log::Write(Log::Error, "Consistency check failed for file: %s", journalFile.value.c_str());
			return false;
		}
	}
	return true;
}

ESErrorCode Worker::handleHostMessage(const ESHeader* header) {
	switch (header->type) {
		case REQ_SHUTDOWN:
			Log::Write(Log::Info, "Host is shutting down");
			stop();
			return ESERR_NO_ERROR;
		case REQ_NEW_CONNECTION:
			return newConnection(header);
		case REQ_CLOSED_CONNECTION:
			return closeConnection(header);
		default:
			return ESERR_NO_ERROR;
	}
}

ESErrorCode Worker::handleMessage(ESHeader* header, const AttachedConnection* connection, ByteBuffer* memory) {
	ESErrorCode err = ESERR_NO_ERROR;
	switch (header->type) {
		case REQ_NEW_TRANSACTION:
			err = newTransaction(header, connection, memory);
			break;
		case REQ_COMMIT_TRANSACTION:
			err = commitTransaction(header, connection, memory);
			break;
		case REQ_ROLLBACK_TRANSACTION:
			err = rollbackTransaction(header, connection, memory);
			break;
		case REQ_READ_JOURNAL:
			err = readJournal(header, connection, memory);
			break;
		case REQ_JOURNAL_EXISTS:
			err = checkIfJournalExists(header, connection, memory);
			break;
		default:
			Log::Write(Log::Warn, "Unknown type: %d", header->type);
			break;
	}

	return err;
}

ESErrorCode Worker::newConnection(const ESHeader* header) {
	mutex_t m;
	SOCKET newSocket = mIpcChild.acceptSharedSocket(&m);
	if (newSocket == INVALID_SOCKET)
		return ESERR_PROCESS_ATTACH_SHARED_SOCKET;

	mAttachedSockets.add(header->client, newSocket, m);
	Log::Write(Log::Info, "New client %d mapped to %d on child process", header->client, newSocket);
	return ESERR_NO_ERROR;
}

ESErrorCode Worker::closeConnection(const ESHeader* header) {
	mAttachedSockets.remove(header->client);
	Log::Write(Log::Info, "Client %d unmapped from child process", header->client);
	return ESERR_NO_ERROR;
}

ESErrorCode Worker::newTransaction(const ESHeader* header, const AttachedConnection* connection, ByteBuffer* memory) {
	assert(header != nullptr);
	assert(connection != nullptr);
	assert(memory != nullptr);

	auto request = memory->allocate<NewTransaction::Request>();

	// Get journal name and make sure that it's valid
	Path journalName;
	auto err = readAndValidatePath(request->journalStringLength, memory, &journalName);
	Log::Write(Log::Debug, "Creating new transaction for journal: %s", journalName.value.c_str());
	if (err != ESERR_NO_ERROR) {
		return err;
	}

	// Retrieve the journal and then create a transaction for it
	auto journal = mJournals.getOrCreate(journalName);
	auto transaction = journal->openTransaction();
	if (transaction.IsInvalid()) {
		return ESERR_JOURNAL_PATH_INVALID;
	}

	// Write the response
	const NewTransaction::Header responseHeader(header->requestUID, id());
	const NewTransaction::Response response(journal->journalSize(), transaction);
	memory->reset();
	memory->write(&responseHeader);
	memory->write(&response);

	// Send the data to the client
	return sendBytesToClient(connection, memory);
}

ESErrorCode Worker::commitTransaction(const ESHeader* header, const AttachedConnection* connection,
                                      ByteBuffer* memory) {
	assert(header != nullptr);
	assert(connection != nullptr);
	assert(memory != nullptr);

	const auto request = memory->allocate<CommitTransaction::Request>();

	// Get journal name and make sure that it's valid
	Path journalName;
	auto err = readAndValidatePath(request->journalStringLength, memory, &journalName);
	if (err != ESERR_NO_ERROR) {
		return err;
	}

	// Retrieve the journal and then create a transaction for it
	auto journal = mJournals.getOrNull(journalName);
	if (journal == nullptr) {
		return ESERR_JOURNAL_IS_CLOSED;
	}

	// Load types that the client sent to us
	const auto typeString = MutableString(request->typeSize, memory);

	// Convert the types into bit masks, used to identify different event types
	vector<string> typeStrings;
	string tmp;
	const char* str = typeString.str;
	const char* end = typeString.str + typeString.length;
	for (; str != end; ++str) {
		if (*str != EVENT_TYPE_DELIMITER)
			tmp += *str;
		else {
			typeStrings.push_back(tmp);
			tmp.clear();
		}
	}
	if (tmp.length() > 0) {
		typeStrings.push_back(tmp);
	}

	const auto types = transactionTypes(typeStrings);
	auto events = MutableString(request->eventsSize, memory);

	// Commit the data into the journal. If the journal is null then it's been garbage collected (i.e. you are 
	// not allowed to have a transaction open for over 1 minute)
	err = journal->tryCommit(request->transactionUID, types, events);
	if (err == ESERR_JOURNAL_TRANSACTION_DOES_NOT_EXIST) {
		return err;
	}

	// Send response
	const auto commitSuccess = err != ESERR_JOURNAL_TRANSACTION_CONFLICT ? 1 : 0;
	const CommitTransaction::Header responseHeader(header->requestUID, id());
	const CommitTransaction::Response response(commitSuccess, journal->journalSize());
	memory->reset();
	memory->write(&responseHeader);
	memory->write(&response);

	// Send the data to the client
	return sendBytesToClient(connection, memory);
}

ESErrorCode Worker::rollbackTransaction(const ESHeader* header, const AttachedConnection* connection,
                                        ByteBuffer* memory) {
	assert(header != nullptr);
	assert(connection != nullptr);
	assert(memory != nullptr);

	// Read the request from the socket
	const auto request = memory->allocate<RollbackTransaction::Request>();

	// Get journal name and make sure that it's valid
	Path journalName;
	auto err = readAndValidatePath(request->journalStringLength, memory, &journalName);
	if (err != ESERR_NO_ERROR) {
		return err;
	}

	// Rollback the transaction
	auto journal = mJournals.getOrNull(journalName);
	if (journal == nullptr) return ESERR_JOURNAL_IS_CLOSED;
	journal->rollback(request->transactionUID);

	// Write the response
	const RollbackTransaction::Header responseHeader(header->requestUID, id());
	const RollbackTransaction::Response response(1);
	memory->reset();
	memory->write(&responseHeader);
	memory->write(&response);

	// Send the data to the client
	return sendBytesToClient(connection, memory);
}

ESErrorCode Worker::readJournal(const ESHeader* header, const AttachedConnection* connection, ByteBuffer* memory) {
	assert(header != nullptr);
	assert(connection != nullptr);
	assert(memory != nullptr);

	const auto requestUID = header->requestUID;

	// Read the request from the socket
	const auto request = memory->allocate<ReadJournal::Request>();
	const auto includeTimestamp = Bits::IsSet(header->properties, ESPROP_INCLUDE_TIMESTAMP);

	// Get journal name and make sure that it's valid
	Path journalName;
	auto err = readAndValidatePath(request->journalStringLength, memory, &journalName);
	if (err != ESERR_NO_ERROR) return err;

	const auto offset = request->offset;
	const auto journalSize = request->journalSize;

	// Client is not allowed to read a journal with a larger offset than it's assumed max length
	if (offset > journalSize) return ESERR_JOURNAL_READ;

	// Rollback the transaction
	auto journal = mJournals.getOrNull(journalName);
	if (journal == nullptr) return ESERR_JOURNAL_IS_CLOSED;

	// Journal size (No not include EOF-marker)
	const auto clampedJournalSize = journalSize > journal->journalSize() ? journal->journalSize() : journalSize;
	uint32_t readBytes = (clampedJournalSize - offset);
	if (readBytes > 0) readBytes--;

	// The amount of bytes left after the header and the response header is written to buffer
	const auto BYTES_LEFT_AFTER_HEADERS =
			mConfig.maxBufferSize - sizeof(ReadJournal::Header) - sizeof(ReadJournal::Response);
	if (readBytes <= BYTES_LEFT_AFTER_HEADERS) {
		// Write and send header and the read-journal responses first
		memory->reset();
		const ReadJournal::Header responseHeader(requestUID, ESPROP_NONE, id());
		memory->write(&responseHeader);
		auto const response = memory->allocate<ReadJournal::Response>();
		response->bytes = 0;

		// Write the journal body if one exists
		if (readBytes > 0) {
			auto stream = AutoClosable<FileInputStream>(journal->inputStream(offset));

			// Write the journal body (with or without timestamp)
			if (includeTimestamp) {
				err = stream->readBytes(memory, readBytes);
				if (isError(err)) return err;
			} else {
				err = stream->readJournalBytes(memory, readBytes, &response->bytes);
				if (isError(err)) return err;
			}
		}

		// Send the data to the client
		return sendBytesToClient(connection, memory);
	} else {
		auto stream = AutoClosable<FileInputStream>(journal->inputStream(offset));
		return readJournalParts(connection, requestUID, includeTimestamp, stream.get(), memory);
	}
}

ESErrorCode Worker::readJournalParts(const AttachedConnection* connection, uint32_t requestUID, bool includeTimestamp,
                                     FileInputStream* stream, ByteBuffer* memory) {
	assert(connection != nullptr);
	assert(memory != nullptr);

	// The amount of bytes left after the header and the response header is written to buffer
	const auto BYTES_LEFT_AFTER_HEADERS =
			mConfig.maxBufferSize - sizeof(ReadJournal::Header) - sizeof(ReadJournal::Response);

	// TODO: Put this as a threaded job (to ensure that smaller journals can be loaded)
	while (stream->bytesLeft() > 0) {
		const uint32_t bytesLeft = stream->bytesLeft();
		const ESHeaderProperties properties = bytesLeft > BYTES_LEFT_AFTER_HEADERS ? ESPROP_MULTIPART : ESPROP_NONE;
		const uint32_t sendSize = bytesLeft > BYTES_LEFT_AFTER_HEADERS ? BYTES_LEFT_AFTER_HEADERS : bytesLeft;

		// Write and send header and the read-journal responses first
		memory->reset();
		memory->ensureCapacity(mConfig.maxBufferSize);
		const ReadJournal::Header responseHeader(requestUID, properties, id());
		memory->write(&responseHeader);
		memory->allocate<ReadJournal::Response>();
		uint32_t bytesWritten = 0;

		// Write the journal body (with or without timestamp)
		if (includeTimestamp) {
			const ESErrorCode err = stream->readBytes(memory, sendSize);
			if (isError(err)) {
				return err;
			}
		} else {
			const ESErrorCode err = stream->readJournalBytes(memory, sendSize, &bytesWritten);
			if (isError(err)) {
				return err;
			}
		}

		// TODO: Make this better!!! Update response header
		auto tm = (ReadJournal::Response*) (memory->ptr() + sizeof(ReadJournal::Header));
		tm->bytes = bytesWritten;

		// Send to client
		const ESErrorCode err = sendBytesToClient(connection, memory);
		if (isError(err)) {
			return err;
		}
	}

	return ESERR_NO_ERROR;
}

ESErrorCode Worker::checkIfJournalExists(const ESHeader* header, const AttachedConnection* connection,
                                         ByteBuffer* memory) {
	assert(header != nullptr);
	assert(connection != nullptr);
	assert(memory != nullptr);

	// Read the request from the socket
	const auto request = memory->allocate<JournalExists::Request>();

	// Get journal name and make sure that it's valid
	Path journalName;
	auto err = readAndValidatePath(request->journalStringLength, memory, &journalName);
	if (err != ESERR_NO_ERROR) {
		return err;
	}

	// Retrieve the journal and then create a transaction for it
	auto journal = mJournals.getOrCreate(journalName);

	// Write the response
	const JournalExists::Header responseHeader(header->requestUID, id());
	const JournalExists::Response response(journal->exists());
	memory->reset();
	memory->write(&responseHeader);
	memory->write(&response);

	// Okay!
	return sendBytesToClient(connection, memory);
}

Bits::Type Worker::transactionTypes(vector<string>& types) {
	Bits::Type transactionType = Bits::None;
	for (auto& type : types) {
		auto mask = mTransactionTypes.find(type);
		if (mask == mTransactionTypes.end()) {
			transactionType |= mNextTransactionTypeBit;
			mTransactionTypes.insert(make_pair(type, mNextTransactionTypeBit));
			mNextTransactionTypeBit = mNextTransactionTypeBit << 1u;
		} else {
			transactionType |= mask->second;
		}
	}
	return transactionType;
}

ESHeader* Worker::loadHeaderFromHost(ByteBuffer* memory) {
	assert(memory != nullptr);

	// Reset the position of the memory
	memory->reset();

	// Get a memory block for the header
	ESHeader* header = memory->allocate<ESHeader>();

	// Make sure to keep the position where the body starts
	memory->memorize();

	// Read the header from host process
	if (mIpcChild.read((char*) header, sizeof(ESHeader)) != sizeof(ESHeader)) {
		return &INVALID_HEADER;
	}

	// Validate request
	if (header->size > (int32_t) mConfig.maxBufferSize) {
		return &INVALID_HEADER;
	}

	// Load the request body
	if (header->size > 0) {
		if (mIpcChild.read(memory->allocate(header->size), header->size) != header->size) {
			return &INVALID_HEADER;
		}
	}

	// Restore to the body position
	memory->restore();

	// Do not allow multipart requests yet!
	if (header->properties != ESPROP_NONE) {
		return &INVALID_HEADER;
	}
	return header;
}

ESErrorCode Worker::readAndValidatePath(const uint32_t length, ByteBuffer* memory, Path* path) {
	// Validate path length
	if (length < 2 || length > 1024) {
		return ESERR_JOURNAL_PATH_INVALID;
	}

	// Validate that the path is absolute
	const char* ptr = memory->allocate(length);
	if (*ptr != '/') {
		return ESERR_JOURNAL_PATH_INVALID;
	}

	// Make the memory block into a std string that does not contain the '/' character.
	static const string logSuffix(".log");
	string logFilename = string(ptr + 1, length - 1) + logSuffix;

	// Ensure that no illegal characters is part of the journal path
	if (logFilename.find("..") != -1 || logFilename.find("//") != -1) {
		return ESERR_JOURNAL_PATH_INVALID;
	}

	*path = Path(logFilename);
	return ESERR_NO_ERROR;
}
