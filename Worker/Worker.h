#ifndef _EVERSTORE_WORKER_H_
#define _EVERSTORE_WORKER_H_

#include "../Shared/everstore.h"
#include "Journals.h"
#include "AttachedSockets.h"

struct Worker : IpcChild {

	Worker(ChildProcessId childProcessId, const Properties& properties);

	~Worker();

	ESErrorCode start();

	void stop();

private:

	ESErrorCode initialize();

	void release();

	bool performConsistencyCheck();

	ESErrorCode handleHostMessage(const ESHeader* header);

	ESErrorCode handleMessage(ESHeader* header, const AttachedConnection* socket, Bytes* memory);

	ESErrorCode newConnection(const ESHeader* header);

	ESErrorCode closeConnection(const ESHeader* header);

	ESErrorCode newTransaction(const ESHeader* header, const AttachedConnection* socket, Bytes* memory);

	ESErrorCode commitTransaction(const ESHeader* header, const AttachedConnection* socket, Bytes* memory);

	ESErrorCode rollbackTransaction(const ESHeader* header, const AttachedConnection* socket, Bytes* memory);

	ESErrorCode readJournal(const ESHeader* header, const AttachedConnection* socket, Bytes* memory);

	ESErrorCode checkIfJournalExists(const ESHeader* header, const AttachedConnection* socket, Bytes* memory);

	// Read and send the journal as multiple responses
	ESErrorCode readJournalParts(const AttachedConnection* socket, uint32_t requestUID,
		bool includeTimestamp, FileInputStream* stream, Bytes* memory);

	// Convert the types into transaction types
	bit_mask transactionTypes(vector<string>& types);

	// Load the next header form host application - with the associated request data
	ESHeader* loadHeaderFromHost(Bytes* memory);

	// Send the supplied memory block to the client.
	ESErrorCode sendBytesToClient(const AttachedConnection* connection, const Bytes* memory);

	// Load a path from the bytes block
	ESErrorCode readAndValidatePath(const uint32_t length, Bytes* memory, _OUT string* s);

private:
	atomic_bool mRunning;
	Journals mJournals;
	AttachedSockets mAttachedSockets;

	// Transaction types
	bit_mask mNextTransactionTypeBit;
	unordered_map<string, bit_mask> mTransactionTypes;

	const Properties mProperties;
};

#endif
