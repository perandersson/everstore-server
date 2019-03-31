//
// Copyright (c) 2019 West Coast Code AB. All rights reserved.
//

#ifndef EVERSTORE_1MUTEX_HPP
#define EVERSTORE_1MUTEX_HPP

#if defined(_WIN32)

#include "Win32/Win32Mutex.hpp"

#else

#include "Unix/UnixMutex.hpp"

#endif

class Process;

/**
 * A specific mutex type used when we a mutex that can be shared between multiple processes
 */
class Mutex
{
public:
	~Mutex();

	/**
	 * @param timeout
	 * @return
	 */
	inline ESErrorCode Lock(uint32_t timeout = 30000u);

	/**
	 *
	 * @return
	 */
	inline ESErrorCode Unlock();

	/**
	 * Share this mutex with the supplied process
	 *
	 * @param process The process we are sharing this mutex with
	 */
	void ShareWith(Process* process);

	/**
	 * Destroy this mutex's internal resources. This is automatically called in the destructor
	 */
	void Destroy();

	/**
	 * Create a new mutex
	 *
	 * @param name The name of the mutex
	 * @return
	 */
	static Mutex* Create(const string& name);

	/**
	 * Get a shared mutex
	 *
	 * @param name
	 * @return
	 */
	static Mutex* LoadFromProcess(const string& name);

private:
	Mutex(const string& name, bool onHost, OsMutex* mutex);

private:
	const string mName;
	const bool mOnHost;
	OsMutex* mMutex;
};


#endif //EVERSTORE_MUTEX_HPP
