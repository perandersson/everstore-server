#include "ESRequestType.h"

namespace
{
	const char* strings[IPC_REQ_COUNT] = {
			"REQ_INVALID",
			"REQ_ERROR",
			"REQ_CLIENT_TYPES",
			"REQ_AUTHENTICATE",
			"REQ_NEW_TRANSACTION",
			"REQ_COMMIT_TRANSACTION",
			"REQ_ROLLBACK_TRANSACTION",
			"REQ_READ_JOURNAL",
			"REQ_JOURNAL_EXISTS",
			"REQ_SERVER_TYPES",
			"REQ_SHUTDOWN",
			"REQ_STATUS",
			"REQ_NEW_CONNECTION",
			"REQ_CLOSED_CONNECTION",
	};

	const char* unknown = "UNKNOWN";
}

bool isExternalRequestType(ESRequestType type) {
	return type > REQ_CLIENT_TYPES && type < REQ_SERVER_TYPES;
}

bool isInternalRequestType(ESRequestType type) {
	return type > REQ_SERVER_TYPES && type < IPC_REQ_COUNT;
}

bool isRequestTypeInitiallyForHost(ESRequestType type) {
	return type == REQ_AUTHENTICATE || type == REQ_NEW_TRANSACTION || type == REQ_JOURNAL_EXISTS;
}

bool isRequestTypeValid(ESRequestType type) {
	return type > REQ_INVALID && type < IPC_REQ_COUNT;
}

const char* parseRequestType(ESRequestType type) {
	if (type >= IPC_REQ_COUNT) {
		return unknown;
	}
	return strings[(uint32_t) type];
}
