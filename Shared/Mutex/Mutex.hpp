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
 * A specific mutex type designed for being shared between multiple processes
 */
class Mutex
{
public:
	~Mutex();

	/**
	 * @param timeout
	 * @return
	 */
	ESErrorCode Lock(uint32_t timeout = 30000u);

	/**
	 *
	 * @return
	 */
	ESErrorCode Unlock();

	/**
	 * Share this mutex with the supplied process
	 *
	 * @param process The process we are sharing this mutex with
	 */
	ESErrorCode ShareWith(Process* process);

	/**
	 * Destroy this mutex's internal resources. This is automatically called in the destructor
	 */
	void Destroy();

	/**
	 * @return <code>true</code> if this mutex is destroyed
	 */
	inline bool IsDestroyed() const { return OsMutex::IsInvalid(&mMutex); }

	/**
	 * Check to see if the supplied mutex is destroyed
	 *
	 * @param m The mutex we want to validate
	 * @return
	 */
	static bool IsDestroyed(Mutex* m) { return m == nullptr || OsMutex::IsInvalid(&m->mMutex); }

	/**
	 * Create a new mutex
	 *
	 * @param name The name of the mutex
	 * @return
	 */
	static Mutex* Create(const string& name);

	/**
	 * Create a new mutex associated with a parent process. Useful when we want to accept a shared process
	 * from
	 *
	 * @param process
	 * @return
	 */
	static Mutex* LoadFromProcess(Process* process);

private:
	Mutex();

private:
	OsMutex mMutex;
};


#endif //EVERSTORE_MUTEX_HPP
