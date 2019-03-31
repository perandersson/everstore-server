#include "Win32Mutex.h"
#include "../ESErrorCodes.h"

mutex_t mutex_create(const string&) {
	SECURITY_ATTRIBUTES sec;
	sec.bInheritHandle = TRUE;
	sec.lpSecurityDescriptor = nullptr;
	sec.nLength = sizeof(SECURITY_ATTRIBUTES);

	return CreateMutexEx(&sec, nullptr, 0, MUTEX_ALL_ACCESS);
}

void mutex_destroy(mutex_t lock) {
	if (lock != INVALID_LOCK) {
		CloseHandle(lock);
	}
}

ESErrorCode mutex_lock(mutex_t lock) {
	assert(lock != INVALID_LOCK);
	const DWORD ret = WaitForSingleObject(lock, 30000L);
	assert(ret == WAIT_OBJECT_0);
	return ESERR_NO_ERROR;
}

ESErrorCode mutex_unlock(mutex_t lock) {
	assert(lock != INVALID_LOCK);
	BOOL ret = ReleaseMutex(lock);
	assert(ret);
	return ESERR_NO_ERROR;
}
