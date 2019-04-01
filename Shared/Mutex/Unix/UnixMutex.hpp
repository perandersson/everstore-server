//
// Copyright (c) 2019 West Coast Code AB. All rights reserved.
//

#ifndef EVERSTORE_UNIXMUTEX_HPP
#define EVERSTORE_UNIXMUTEX_HPP

#include <pthread.h>
#include "../../ESErrorCodes.h"

struct OsProcess;

struct OsMutex
{
    pthread_mutex_t* ptr;
    char name[64];
    bool onHost;

    static ESErrorCode Create(const string& name, OsMutex* mutex);

    static ESErrorCode Destroy(OsMutex* mutex);

    static ESErrorCode Lock(OsMutex* mutex, uint32_t timeout);

    static ESErrorCode Unlock(OsMutex* mutex);

    static ESErrorCode ShareWith(OsMutex* mutex, OsProcess* process);

    static ESErrorCode LoadFromProcess(OsMutex* mutex, OsProcess* process);

    inline static bool IsInvalid(const OsMutex* mutex) { return mutex == nullptr || mutex->ptr == nullptr; }
};


#endif //EVERSTORE_UNIXMUTEX_HPP
