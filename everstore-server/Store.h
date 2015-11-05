#ifndef _EVERSTORE_STORE_H_
#define _EVERSTORE_STORE_H_

#include <everstore.h>
#include "StoreServer.h"

struct Store {

	Store(const Properties& props);

	~Store();

	//
	// Starts the event store
	ESErrorCode start();

	//
	// Stop the event store
	void stop();

private:
	//
	// Initialize the event store
	ESErrorCode initialize();

	// 
	// Release the event store's internal memory
	void release();

	//
	// Perform consistency check on the journals located on the HDD
	bool performConsistencyCheck();

private:
	const Properties& mProperties;
	atomic_bool mRunning;
	IpcHost* mHost;
	StoreServer* mServer;
	Authenticator* mAuthenticator;
	vector<thread> mClientConnections;
};

#endif
