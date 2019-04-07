//
// Copyright (c) 2019 West Coast Code AB. All rights reserved.
//

#include "UnixMutex.hpp"
#include "../../Process/Process.hpp"
#include "../../Log/Log.hpp"
#include <sys/mman.h>
#include <sys/fcntl.h>

ESErrorCode OsMutex::Create(const string& name, OsMutex* mutex) {
	memset(mutex, 0, sizeof(OsMutex));
	const string sharedMemoryName =
#ifdef __APPLE__
			string("/tmp/everstore/") + name;
#else
	string("/") + name;
#endif

	// Setup the actual mutex object
	mutex->ptr = nullptr;
	mutex->onHost = true;
	memset(mutex->name, 0, sizeof(mutex->name));
	strncpy(mutex->name, sharedMemoryName.c_str(), sharedMemoryName.size());

	// Give us a handle used for share memory access - used for inter-process support of the mutex
#ifdef __APPLE__
	int shm_fd = open(mutex->name, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
#else
	int shm_fd = shm_open(mutex->name, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
#endif
	if (shm_fd == -1) {
		Log::Write(Log::Error, "UnixMutex | %s failed to open share memory handle", mutex->name);
		return ESERR_MUTEX_CREATE;
	}

	// Use truncate as a way to set the size of the shared memory to a specific amount
	if (ftruncate(shm_fd, sizeof(pthread_mutex_t)) == -1) {
		close(shm_fd);
		shm_unlink(mutex->name);
		return ESERR_MUTEX_CREATE;
	}

	// Map the memory to the mutex
	mutex->ptr = (pthread_mutex_t*) mmap(nullptr, sizeof(pthread_mutex_t), PROT_READ | PROT_WRITE, MAP_SHARED,
	                                     shm_fd, 0);

	// From documentation "http://man7.org/linux/man-pages/man3/shm_open.3.html":
	// After a call to mmap(2) the file descriptor may be closed without affecting the
	// memory mapping.
	close(shm_fd);

	if (mutex->ptr == nullptr) {
		Log::Write(Log::Error, "UnixMutex | No shared memory received for %s ", mutex->name);
		shm_unlink(mutex->name);
		return ESERR_MUTEX_CREATE;
	}
	memset(mutex->ptr, 0, sizeof(pthread_mutex_t));

	// Initialize shared mutex model
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
#ifndef _POSIX_THREAD_PROCESS_SHARED
#error "This platform does not support process shared mutex!"
#else
	auto ret = pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
	if (ret != 0) {
		Log::Write(Log::Error, "UnixMutex | %s failed on pthread_mutexattr_setpshared", mutex->name);
		return ESERR_MUTEX_CREATE;
	}
#endif
	pthread_mutex_init(mutex->ptr, &attr);
	pthread_mutexattr_destroy(&attr);
	return 0;
}

ESErrorCode OsMutex::Destroy(OsMutex* mutex) {
	// Verify if the mutex is already destroyed
	if (IsInvalid(mutex)) {
		return ESERR_MUTEX_DESTROYED;
	}

	// Release the mutexes internal memory if on the host. This is only needed to be done once, and should thus be done
	// by the application using the mutex last (the server and not the worker)
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

	if (OsProcess::IsInvalid(process)) {
		Log::Write(Log::Error, "UnixMutex | Failed to share %s over UnixSocket(%d)", mutex->name, process->unixSocket);
		return ESERR_INVALID_ARGUMENT;
	}

	// Send the name of the lock to the sub-process. It's up to the child-process to attach itself to the mutex
	const auto writtenBytes = OsProcess::Write(process, mutex->name, sizeof mutex->name);
	if (writtenBytes != sizeof mutex->name) {
		return ESERR_PIPE_WRITE;
	}

	return ESERR_NO_ERROR;
}

ESErrorCode OsMutex::LoadFromProcess(OsMutex* mutex, OsProcess* process) {
	if (mutex == nullptr) {
		return ESERR_INVALID_ARGUMENT;
	}

	if (OsProcess::IsInvalid(process)) {
		return ESERR_PROCESS_DESTROYED;
	}
	memset(mutex, 0, sizeof(OsMutex));

	// Read the name of the mutex so that we can attach it to this process
	const auto readBytes = OsProcess::Read(process, mutex->name, sizeof mutex->name);
	if (readBytes != sizeof mutex->name) {
		Log::Write(Log::Error, "OsMutex | Failed to read from UnixSocket(%d)", process->unixSocket);
		return ESERR_PIPE_READ;
	}

	// Open the shared memory. It should already exist because it should've been created by the parent process
#ifdef __APPLE__
	int shm_fd = open(mutex->name, O_RDWR, S_IRUSR | S_IWUSR);
#else
	int shm_fd = shm_open(mutex->name, O_RDWR, S_IRUSR | S_IWUSR);
#endif
	if (shm_fd == -1) {
		Log::Write(Log::Error, "UnixMutex | %s failed to open share memory handle", mutex->name);
		return ESERR_MUTEX_ATTACH;
	}

	// Map the memory to the mutex
	mutex->ptr = (pthread_mutex_t*) mmap(nullptr, sizeof(pthread_mutex_t), PROT_READ | PROT_WRITE, MAP_SHARED,
	                                     shm_fd, 0);

	// From documentation "http://man7.org/linux/man-pages/man3/shm_open.3.html":
	// After a call to mmap(2) the file descriptor may be closed without affecting the
	// memory mapping.
	close(shm_fd);

	if (mutex->ptr == nullptr) {
		Log::Write(Log::Error, "UnixMutex | No shared memory received for %s ", mutex->name);
		shm_unlink(mutex->name);
		return ESERR_MUTEX_ATTACH;
	}

	return ESERR_NO_ERROR;
}
