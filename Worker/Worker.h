#ifndef _EVERSTORE_WORKER_H_
#define _EVERSTORE_WORKER_H_

#include "../Shared/everstore.h"
#include "Journals.h"
#include "AttachedSockets.h"
#include "../Shared/Ipc/IpcChild.h"

class Worker
{
public:
	Worker(ProcessID id, const Config& config);

	~Worker();

	ESErrorCode start();

	void stop();

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
	Bits::Type transactionTypes(vector<string>& types);

	// Load the next header form host application - with the associated request data
	ESHeader* loadHeaderFromHost(ByteBuffer* memory);

	// Send the supplied memory block to the client.
	ESErrorCode sendBytesToClient(const AttachedConnection* connection, const ByteBuffer* memory);

	// Load a path from the bytes block
	ESErrorCode readAndValidatePath(uint32_t length, ByteBuffer* memory, Path* path);

	// Retrieves this child's unique id
	inline const ProcessID id() const { return mId; }

	inline Process* process() { return mIpcChild->process(); }

private:
	const ProcessID mId;
	IpcChild* mIpcChild;
	atomic_bool mRunning;
	Journals mJournals;
	AttachedSockets mAttachedSockets;

	// Transaction types
	Bits::Type mNextTransactionTypeBit;
	unordered_map<string, Bits::Type> mTransactionTypes;

	const Config mConfig;
};

#endif
