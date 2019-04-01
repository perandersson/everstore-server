#ifndef _EVERSTORE_ERROR_CODES_H_
#define _EVERSTORE_ERROR_CODES_H_

#include "es_config.h"

// Error code
typedef uint32_t ESErrorCode;

// Fatal error codes
//
// When a fatal error occures then a gracefull shutdown of the server should happen.
enum ESFatalErrorCodes : ESErrorCode {
	ESERR_FATAL = 1000,

	ESERR_FILESYSTEM_CHANGEPATH,

	ESERR_SOCKET,
	ESERR_SOCKET_INIT,
	ESERR_SOCKET_CONFIGURE,
	ESERR_SOCKET_BIND,
	ESERR_SOCKET_LISTEN,
	ESERR_SOCKET_ACCEPT,
	ESERR_SCCKET_DESTROYED,

	ESERR_PROCESS_CREATE_CHILD,
	ESERR_PROCESS_DESTROYED,
	ESSER_PROCESS_SHARE_SOCKET,
	ESERR_PROCESS_ATTACH_SHARED_SOCKET,
	ESERR_PROCESS_FAILED_TO_CLOSE,
	ESERR_PROCESS_FAILED_TO_START,
	
	ESERR_PIPE_READ,
	ESERR_PIPE_WRITE,
	ESERR_PIPE_CREATE,
	ESERR_PIPE_LISTEN,
	ESERR_PIPE_CONNECT,

	ESERR_FILE_LOCK_FAILED,

	ESERR_MUTEX_SHARE,
	ESERR_MUTEX_ATTACH,
	ESERR_MUTEX_LOCK_FAILED,
	ESERR_MUTEX_UNLOCK_FAILED,
	
	ESERR_STORE_ALREADY_RUNNING,
	ESERR_STORE_CONSISTENCY_CHECK_FAILED,

	ESERR_CLIENT_MUTEX_FAILED,

	ESERR_REQUEST_TYPE_UNKNOWN,

	ESERR_FATAL_COUNT
};

// Error codes that occures due to client problems. 
// 
// When this type of problem occurs then a gracefull disconnect of the client should happen.
enum ESErrorCodes : ESErrorCode {
	ESERR_NO_ERROR = 0,
	ESERR_UNKNOWN,

	ESERR_AUTHENTICATION_FAILED,

	ESERR_WORKER_UNKNOWN,

	ESERR_JOURNAL_IS_CLOSED,
	ESERR_JOURNAL_READ,
	ESERR_JOURNAL_TRANSACTION_DOES_NOT_EXIST,
	ESERR_JOURNAL_TRANSACTION_CONFLICT,
	ESERR_JOURNAL_PATH_INVALID,

	ESERR_SOCKET_SEND,
	ESERR_SOCKET_DISCONNECTED,
	ESERR_SOCKET_NOT_ATTACHED,

	ESERR_MUTEX_ALREADY_DESTROYED,

	ESERR_COUNT,
};

static_assert((uint32_t)ESERR_COUNT < (uint32_t)ESERR_FATAL,
              "If this fails then it means that the fatal error index is part of the non-fatal error list");

// Convert the supplied error code into a readable string
const char* parseErrorCode(ESErrorCode code);

// Check if the supplied error code is fatal or not
bool isErrorCodeFatal(ESErrorCode code);

// Check if the supplied code is an error
bool isError(ESErrorCode code);

// Check if an error code has occured, but it's not fatal
bool IsErrorButNotFatal(ESErrorCode code);

#endif
