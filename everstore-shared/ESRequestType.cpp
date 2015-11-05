#include "ESRequestType.h"

bool isExternalRequestType(ESRequestType type) {
	return type > REQ_CLIENT_TYPES && type < REQ_SERVER_TYPES;
}

bool isInternalRequestType(ESRequestType type) {
	return type > REQ_SERVER_TYPES && type < IPC_REQ_COUNT;
}

bool isRequestTypeInitiallyForHost(ESRequestType type) {
	return type == REQ_AUTHENTICATE || type == REQ_NEW_TRANSACTION;
}

bool isRequestTypeValid(ESRequestType type) {
	return type > REQ_INVALID && type < IPC_REQ_COUNT;
}
