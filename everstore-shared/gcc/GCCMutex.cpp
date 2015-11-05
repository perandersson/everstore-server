#include "GCCMutex.h"
#include <sys/mman.h>
#include <sys/fcntl.h>

ESErrorCode mutex_attach(const string& name, mutex_t mutex) {
	// Open the shared memory
#ifdef __APPLE__
	const int fd = open(name.c_str(), O_RDWR, S_IRUSR|S_IWUSR);
#else
	const int fd = shm_open(name.c_str(), O_RDWR, S_IRUSR|S_IWUSR);
#endif
	if (fd == 0) return ESERR_MUTEX_SHARE;

	// Map the memory to the mutex
	mutex->ptr = (pthread_mutex_t*)mmap(0, sizeof(pthread_mutex_t), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	mutex->onhost = false;
	close(fd);
	if (mutex->ptr == nullptr) {
		shm_unlink(name.c_str());
		return ESERR_MUTEX_SHARE;
	}

	return ESERR_NO_ERROR;
}

mutex_t mutex_create(const string& name) {
	const string sharedMemoryName =
#ifdef __APPLE__
		string("/tmp/everstore/") + name;
#else
		name;
#endif
	// Create the mutex object
	mutex_t m = (mutex_t) malloc(sizeof(_mutex_t));
	m->ptr = NULL;
	m->onhost = true;
	memset(m->name, 0, sizeof(m->name));
	strncpy(m->name, sharedMemoryName.c_str(), 63);

	// Open shared memory
#ifdef __APPLE__
	const int fd = open(m->name, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
#else
	const int fd = shm_open(m->name, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
#endif
	if (fd == -1) return INVALID_LOCK;
	if(ftruncate(fd, sizeof(pthread_mutex_t)) == -1) {
		shm_unlink(m->name);
		return INVALID_LOCK;
	}

	// Map the memory to the mutex
	m->ptr = (pthread_mutex_t*)mmap(0, sizeof(pthread_mutex_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	memset(m->ptr, 0, sizeof(pthread_mutex_t));
	close(fd);
	if (m->ptr == NULL) {
		shm_unlink(m->name);
		return INVALID_LOCK;
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
	pthread_mutex_init(m->ptr, &attr);
	pthread_mutexattr_destroy(&attr);
	return m;
}

void mutex_destroy(mutex_t l) {
	assert(l != INVALID_LOCK);
	if (l->onhost) {
		pthread_mutex_destroy(l->ptr);
	}
	munmap(l->ptr, sizeof(pthread_mutex_t));
	shm_unlink(l->name);
	free(l);
}

ESErrorCode mutex_lock(mutex_t l) {
	const int ret = pthread_mutex_lock(l->ptr);
	return ret == 0 ? ESERR_NO_ERROR : ESERR_MUTEX_LOCK_FAILED;
}

ESErrorCode mutex_unlock(mutex_t l) {
	const int ret = pthread_mutex_unlock(l->ptr);
	return ret == 0 ? ESERR_NO_ERROR : ESERR_MUTEX_LOCK_FAILED;
}
