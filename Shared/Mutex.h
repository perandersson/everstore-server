#ifndef EVERSTORE_MUTEX_H
#define EVERSTORE_MUTEX_H

#ifdef WIN32

#include "win32/Win32Mutex.h"

#else

#include "gcc/GCCMutex.h"

#endif

#include "ESErrorCodes.h"

mutex_t mutex_create(const string& name);

void mutex_destroy(mutex_t l);

ESErrorCode mutex_lock(mutex_t l);

ESErrorCode mutex_unlock(mutex_t l);


#endif //EVERSTORE_MUTEX_H
