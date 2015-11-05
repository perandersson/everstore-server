#ifndef _EVERSTORE_PROTOCOL_H_
#define _EVERSTORE_PROTOCOL_H_

#include "es_config.h"

enum ESRequestType : uint32_t  {
	REQ_INVALID = 0,
	REQ_ERROR,

	//
	// External request types
	//

	REQ_CLIENT_TYPES,
	REQ_AUTHENTICATE,
	REQ_NEW_TRANSACTION,
	REQ_COMMIT_TRANSACTION,
	REQ_ROLLBACK_TRANSACTION,
	REQ_READ_JOURNAL,

	//
	// Internal request types
	//

	REQ_SERVER_TYPES,
	REQ_SHUTDOWN,
	REQ_STATUS,
	REQ_NEW_CONNECTION,
	REQ_CLOSED_CONNECTION,
	
	IPC_REQ_COUNT
};

bool isExternalRequestType(ESRequestType type);
bool isInternalRequestType(ESRequestType type);
bool isRequestTypeInitiallyForHost(ESRequestType type);
bool isRequestTypeValid(ESRequestType type);


#endif
