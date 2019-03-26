#ifndef _EVERSTORE_WORKER_H_
#define _EVERSTORE_WORKER_H_

#include "../Shared/everstore.h"
#include "Journals.h"
#include "AttachedSockets.h"

class Worker
{
public:
	Worker(ChildProcessId childProcessId, const Config& config);

	~Worker();

	ESErrorCode start();

	void stop();

	// Logging
	void log(const char* str, ...);

	// Logging
	void error(const char* str, ...);

	// Log an error message
	void error(ESErrorCode err);

private:

	ESErrorCode initialize();

	void release();

	bool performConsistencyCheck();

	ESErrorCode handleHostMessage(const ESHeader* header);

	ESErrorCode handleMessage(ESHeader* header, const AttachedConnection* socket, ByteBuffer* memory);

	ESErrorCode newConnection(const ESHeader* header);

	ESErrorCode closeConnection(const ESHeader* header);

	ESErrorCode newTransaction(const ESHeader* header, const AttachedConnection* socket, ByteBuffer* memory);

	ESErrorCode commitTransaction(const ESHeader* header, const AttachedConnection* socket, ByteBuffer* memory);

	ESErrorCode rollbackTransaction(const ESHeader* header, const AttachedConnection* socket, ByteBuffer* memory);

	ESErrorCode readJournal(const ESHeader* header, const AttachedConnection* socket, ByteBuffer* memory);

	ESErrorCode checkIfJournalExists(const ESHeader* header, const AttachedConnection* socket, ByteBuffer* memory);

	// Read and send the journal as multiple responses
	ESErrorCode readJournalParts(const AttachedConnection* socket, uint32_t requestUID,
	                             bool includeTimestamp, FileInputStream* stream, ByteBuffer* memory);

	// Convert the types into transaction types
	bit_mask transactionTypes(vector<string>& types);

	// Load the next header form host application - with the associated request data
	ESHeader* loadHeaderFromHost(ByteBuffer* memory);

	// Send the supplied memory block to the client.
	ESErrorCode sendBytesToClient(const AttachedConnection* connection, const ByteBuffer* memory);

	// Load a path from the bytes block
	ESErrorCode readAndValidatePath(uint32_t length, ByteBuffer* memory, _OUT string* s);

	// Retrieves this child's unique id
	inline const ChildProcessId id() const { return mIpcChild.id(); }

	inline process_t* process() { return mIpcChild.process(); }

private:
	IpcChild mIpcChild;
	atomic_bool mRunning;
	Journals mJournals;
	AttachedSockets mAttachedSockets;

	// Transaction types
	bit_mask mNextTransactionTypeBit;
	unordered_map<string, bit_mask> mTransactionTypes;

	const Config mConfig;
};

#endif
