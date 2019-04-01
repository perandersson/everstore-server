#include "ESErrorCodes.h"

const char* _ES_FATAL_ERROR_CODE_STRING[ESERR_FATAL_COUNT - ESERR_FATAL] = {
		"Unknown fatal error.",

		// FileSystem
		"Could not set the current path to the journal directory. Make sure that the journal directory exists.",

		// Socket
		"An unknown socket error has occurred",
		"Could not initialize socket.",
		"Could not configure socket.",
		"Could not bind socket",
		"Failed when trying to listen for connections on socket. Socket might be in use by an already running server instance or another application.",
		"Could not accept incomming connections.",

		// Process
		"Could not create child process. You might be out of memory.",
		"Process is destroyed",
		"Could not share socket from host to client.",
		"Child process failed to inherit socket from host.",
		"Failed to stop the currently running process for some reason",
		"Failed to start a new process",

		// Pipe
		"Could not read data from pipe. Pipe might be closed or the request type was invalid.",
		"Could not write data to pipe. Pipe might be closed.",
		"Failed to create pipe. You might be out of resources or an server instance might already be running.",
		"Could not listen for incoming connections to the pipe. You might be out or resources, or more than one process is trying to use the same pipe",
		"Could not connect to pipe. You might be out or resources, or more than one process is trying to use the same pipe",

		// File
		"Could not create lock file. Make sure that your application has the neccessary system rights to be able to write to the data directory",

		// Mutex
		"Could not create mutex",
		"Mutex is destroyed",
		"Could not share mutex",
		"Could not attach to shared mutex",
		"Could not lock mutex.",
		"Could not unlock mutex.",

		// Store
		"Server is already running. Please close it before performing this action.",
		"Consistency check failed. Please read the 'consistency.log' file located in the server data directory for more information about which journals we failed to fix.", // TODO <----- consistency.log

		// Store Client
		"Could not start store client. Mutex was not initialized properly",

		// Request
		"The supplied request is unknown",
};

const char* _ES_ERROR_CODE_STRING[ESERR_COUNT] = {
		"No error has occured.",
		"Unknown error has occured",

		// Authentication
		"Client authentication failed",

		// Worker
		"The supplied worker does not exists.",

		// Journal
		"The requested journal is closed. Transactions to a journal is not allowed to live for more than one minute.",
		"Could not read journal data. The file might have been changede outside the program.",
		"The supplied transaction does not exist.",
		"Conflict occured when the transaction was committed",
		"The supplied journal path is invalid",

		"Could not send data to client",
		"Socket disconnected",
		"Socket not attached",

		"Mutex is already destroyed",
};

const char* _ES_ERROR_CODE_UNKNOWN = "Unknown error code";

const char* parseErrorCode(ESErrorCode code) {
	if (code >= ESERR_FATAL && code < ESERR_FATAL_COUNT) {
		const uint32_t idx = code - ESERR_FATAL;
		return _ES_FATAL_ERROR_CODE_STRING[idx];
	} else if (code >= ESERR_NO_ERROR && code < ESERR_COUNT) {
		const uint32_t idx = code - ESERR_NO_ERROR;
		return _ES_ERROR_CODE_STRING[idx];
	}
	return _ES_ERROR_CODE_UNKNOWN;
}

bool isErrorCodeFatal(ESErrorCode code) {
	return code >= ESERR_FATAL && code < ESERR_FATAL_COUNT;
}

bool isError(ESErrorCode code) {
	return code > ESERR_NO_ERROR;
}

bool IsErrorButNotFatal(ESErrorCode code) {
	return isError(code) && !isErrorCodeFatal(code);
}
