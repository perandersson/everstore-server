//
// Copyright (c) 2019 West Coast Code AB. All rights reserved.
//

#include "UnixMutex.hpp"
#include "../../Process/Process.hpp"
#include <sys/mman.h>
#include <sys/fcntl.h>

ESErrorCode OsMutex::Create(const string& name, OsMutex* mutex) {
	const string sharedMemoryName =
#ifdef __APPLE__
			string("/tmp/everstore/") + name;
#else
	name;
#endif

	// Setup the actual mutex object
	mutex->ptr = nullptr;
	mutex->onHost = true;
	memset(mutex->name, 0, sizeof(mutex->name));
	strncpy(mutex->name, sharedMemoryName.c_str(), 63);

	// Open shared memory
#ifdef __APPLE__
	const int fd = open(mutex->name, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
#else
	const int fd = shm_open(mutex->name, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
#endif
	if (fd == -1) {
		return ESERR_MUTEX_CREATE;
	}

	// Remove all stored data in the shared memory. This is useful if the memory was used by someone else
	// and not removed properly (i.e. if the application crashes etc.)
	if (ftruncate(fd, sizeof(pthread_mutex_t)) == -1) {
		shm_unlink(mutex->name);
		return ESERR_MUTEX_CREATE;
	}

	// Map the memory to the mutex
	mutex->ptr = (pthread_mutex_t*) mmap(0, sizeof(pthread_mutex_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	memset(mutex->ptr, 0, sizeof(pthread_mutex_t));
	close(fd);
	if (mutex->ptr == nullptr) {
		shm_unlink(mutex->name);
		return ESERR_MUTEX_CREATE;
	}

	// Initialize shared mutex model
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
#ifndef _POSIX_THREAD_PROCESS_SHARED
#error "This platform does not support process shared mutex!"
#else
	auto ret = pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
	assert(ret == 0);
#endif
	pthread_mutex_init(mutex->ptr, &attr);
	pthread_mutexattr_destroy(&attr);
	return 0;
}

ESErrorCode OsMutex::Destroy(OsMutex* mutex) {
	if (IsInvalid(mutex)) {
		return ESERR_MUTEX_DESTROYED;
	}

	// Release the mutexes internal memory if on the host
	if (mutex->onHost) {
		pthread_mutex_destroy(mutex->ptr);
	}

	// Unmap the memory for the mutex
	munmap(mutex->ptr, sizeof(pthread_mutex_t));
	shm_unlink(mutex->name);
	return ESERR_NO_ERROR;
}

ESErrorCode OsMutex::Lock(OsMutex* mutex, uint32_t timeout) {
	if (IsInvalid(mutex)) {
		return ESERR_MUTEX_DESTROYED;
	}
	// TODO OSX do not support pthread_mutex_timedlock. Find an alternate solution for it
	const int ret = pthread_mutex_lock(mutex->ptr);
	return ret == 0 ? ESERR_NO_ERROR : ESERR_MUTEX_LOCK_FAILED;
}

ESErrorCode OsMutex::Unlock(OsMutex* mutex) {
	if (IsInvalid(mutex)) {
		return ESERR_MUTEX_DESTROYED;
	}
	const int ret = pthread_mutex_unlock(mutex->ptr);
	return ret == 0 ? ESERR_NO_ERROR : ESERR_MUTEX_LOCK_FAILED;
}

ESErrorCode OsMutex::ShareWith(OsMutex* mutex, OsProcess* process) {
	if (IsInvalid(mutex)) {
		return ESERR_MUTEX_DESTROYED;
	}

	// Send the name of the lock to the sub-process. It's up to the child-process to attach itself to the mutex
	const auto writtenBytes = OsProcess::Write(process, mutex->name, sizeof mutex->name);
	if (writtenBytes != sizeof mutex->name) {
		return ESERR_PIPE_WRITE;
	}

	return ESERR_NO_ERROR;
}

ESErrorCode OsMutex::LoadFromProcess(OsMutex* mutex, OsProcess* process) {
	// Read the name of the mutex so that we can attach it to this process
	const auto readBytes = OsProcess::Read(process, mutex->name, sizeof mutex->name);
	if (readBytes != sizeof mutex->name) {
		return ESERR_PIPE_READ;
	}

	// Open the shared memory. It should already exist because it should've been created by the parent process
#ifdef __APPLE__
	const int fd = open(mutex->name, O_RDWR, S_IRUSR | S_IWUSR);
#else
	const int fd = shm_open(mutex->name, O_RDWR, S_IRUSR|S_IWUSR);
#endif
	if (fd == 0) {
		return ESERR_MUTEX_ATTACH;
	}

	// Map the memory to the mutex
	mutex->ptr = (pthread_mutex_t*) mmap(0, sizeof(pthread_mutex_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	mutex->onHost = false;
	close(fd);
	if (mutex->ptr == nullptr) {
		shm_unlink(mutex->name);
		return ESERR_MUTEX_ATTACH;
	}

	return ESERR_NO_ERROR;
}
