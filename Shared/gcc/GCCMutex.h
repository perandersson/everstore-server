//
// Created by Per Andersson on 31/07/15.
//

#ifndef EVERSTORE_GCCMUTEX_H
#define EVERSTORE_GCCMUTEX_H

#include "../es_config.h"
#include "../ESErrorCodes.h"
#include <pthread.h>

#define INVALID_LOCK 0

struct _mutex_t {
	pthread_mutex_t* ptr;
	char name[64];
	bool onhost;
};

typedef _mutex_t* mutex_t;

ESErrorCode mutex_attach(const string& name, mutex_t mutex);

#endif //EVERSTORE_GCCMUTEX_H
