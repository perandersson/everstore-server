#ifndef _EVERSTORE_WIN32_PROCESS_H_
#define _EVERSTORE_WIN32_PROCESS_H_

#include "../es_config.h"
#include "../ESErrorCodes.h"

#define INVALID_PROCESS -1
#define INVALID_PIPE -1

#define PIPE int

struct process_t {
	pid_t handle;
	PIPE pipe;
};

#endif
