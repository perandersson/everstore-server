#ifndef _EVERSTORE_WIN32_PROCESS_H_
#define _EVERSTORE_WIN32_PROCESS_H_

#include "../ESErrorCodes.h"

#define INVALID_PIPE INVALID_HANDLE_VALUE
#define PIPE HANDLE

struct process_t {
	PROCESS_INFORMATION processInfo;
	STARTUPINFO startupInfo;
	bool running;
	PIPE pipe;
};

#endif
