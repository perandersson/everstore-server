#ifndef _EVERSTORE_MESSAGES_H_
#define _EVERSTORE_MESSAGES_H_

#include "ESHeader.h"
#include "TransactionId.h"

struct Authentication {
	static const ESRequestType TYPE = REQ_AUTHENTICATE;
	struct Request {
		uint32_t usernameLength;
		uint32_t passwordLength;
	};
	struct Response {
		char success;
	};
};

static_assert(sizeof(Authentication::Request) == 8, "Expected CommitTransaction::Request to be 8 byte(s)");
static_assert(sizeof(Authentication::Response) == 1, "Expected CommitTransaction::Response to be 1 byte(s)");

struct NewTransaction {
	static const ESRequestType TYPE = REQ_NEW_TRANSACTION;
	struct Request {
		uint32_t journalStringLength;	// Length of the journal name
	};
	struct Response {
		uint32_t journalSize;			// The size of the journal when this transaction was created
		uint32_t transactionUID;		// A unique identifier for the current transaction

		Response(uint32_t journalSize, const TransactionId id)
			: journalSize(journalSize), transactionUID(id.value) {}
		~Response() {}
	};
	struct Header : ESHeader {
		Header(uint32_t requestId, ChildProcessId workerId) 
		: ESHeader(TYPE, sizeof(Response), requestId, ESPROP_NONE, workerId) {}
		~Header() {}
	};
};

static_assert(sizeof(NewTransaction::Request) == 4, "Expected CommitTransaction::Request to be 4 byte(s)");
static_assert(sizeof(NewTransaction::Response) == 8, "Expected CommitTransaction::Response to be 8 byte(s)");

// Delimiter for the event types
static const char EVENT_TYPE_DELIMITER = ',';

struct CommitTransaction {
	static const ESRequestType TYPE = REQ_COMMIT_TRANSACTION;
	struct Request {
		uint32_t journalStringLength;	// Length of the journal name
		uint32_t typeSize;				// The byte size for the event types
		uint32_t eventsSize;			// The byte size for the actual events
		uint32_t transactionUID;		// A unique identifier for the current transaction
	};
	struct Response {
		uint32_t success;				// If a conflict occured (TRUE or FALSE)
		uint32_t journalSize;			// The size of the journal when this transaction was created

		Response(uint32_t success, uint32_t journalSize) : success(success), journalSize(journalSize) {}
		~Response() {}
	};
	struct Header : ESHeader {
		Header(uint32_t requestId, ChildProcessId workerId) 
		: ESHeader(TYPE, sizeof(Response), requestId, ESPROP_NONE, workerId) {}
		~Header() {}
	};
};

static_assert(sizeof(CommitTransaction::Request) == 16, "Expected CommitTransaction::Request to be 16 byte(s)");
static_assert(sizeof(CommitTransaction::Response) == 8, "Expected CommitTransaction::Response to be 8 byte(s)");

struct RollbackTransaction {
	static const ESRequestType TYPE = REQ_ROLLBACK_TRANSACTION;
	struct Request {
		uint32_t journalStringLength;	// Length of the journal name
		uint32_t transactionUID;		// Transaction we want to delete
	};
	struct Response {
		char success;

		Response(char success) : success(success) {}
		~Response() {}
	};
	struct Header : ESHeader {
		Header(uint32_t requestId, ChildProcessId workerId) 
		: ESHeader(TYPE, sizeof(Response), requestId, ESPROP_NONE, workerId) {}
		~Header() {}
	};
};

static_assert(sizeof(RollbackTransaction::Request) == 8, "Expected RollbackTransaction::Request to be 8 byte(s)");
static_assert(sizeof(RollbackTransaction::Response) == 1, "Expected RollbackTransaction::Response to be 1 byte(s)");

struct RequestError {
	static const ESRequestType TYPE = REQ_ERROR;
	struct Response {
		ESErrorCode errorCode;

		Response(ESErrorCode errorCode) : errorCode(errorCode) {}
		~Response() {}
	};
	struct Header : ESHeader {
		Header(uint32_t requestId, ChildProcessId workerId) 
		: ESHeader(TYPE, sizeof(Response), requestId, ESPROP_NONE, workerId) {}
		~Header() {}
	};
};

static_assert(sizeof(RequestError::Response) == 4, "Expected RequestError::Request to be 4 byte(s)");

struct ReadJournal {
	static const ESRequestType TYPE = REQ_READ_JOURNAL;
	struct Header : ESHeader {
		Header(uint32_t requestId, ESHeaderProperties properties, ChildProcessId workerId) 
		: ESHeader(TYPE, sizeof(Response), requestId, properties, workerId) {}
		~Header() {}
	};
	struct Request {
		uint32_t journalStringLength;	// Length of the journal name
		uint32_t offset;				// Offset where we want to start read the data from
		uint32_t journalSize;			// The amount of bytes we want to read
	};
	struct Response {
		uint32_t bytes;				// The total amount of bytes sent back the the client

		Response(uint32_t bytes) : bytes(bytes) {}
		~Response() {}
	};
};

static_assert(sizeof(ReadJournal::Request) == 12, "Expected ReadJournal::Request to be 12 byte(s)");
static_assert(sizeof(ReadJournal::Response) == 4, "Expected ReadJournal::Response to be 4 byte(s)");

struct JournalExists {
	static const ESRequestType TYPE = REQ_JOURNAL_EXISTS;
	struct Header : ESHeader {
		Header(uint32_t requestId, ChildProcessId workerId)
		: ESHeader(TYPE, sizeof(Response), requestId, ESPROP_NONE, workerId) {}
		~Header() {}
	};
	struct Request {
		uint32_t journalStringLength; // Length of the journal name
	};
	struct Response {
		char exists;

		Response(bool exists) : exists(exists ? 1 : 0) {}
		~Response() {}
	};
};

static_assert(sizeof(JournalExists::Request) == 4, "Expected JournalExists::Request to be 4 byte(s)");
static_assert(sizeof(JournalExists::Response) == 1, "Expected JournalExists::Response to be 1 byte(s)");

#endif
