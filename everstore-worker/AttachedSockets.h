#ifndef _EVERSTORE_ATTACHED_SOCKETS_H_
#define _EVERSTORE_ATTACHED_SOCKETS_H_

#include <everstore.h>

struct AttachedConnection {
	SOCKET socket;
	mutex_t lock;
};

struct AttachedSockets : unordered_map<SOCKET, AttachedConnection*> {

	AttachedSockets();

	~AttachedSockets();

	AttachedConnection* get(SOCKET s);

	void add(SOCKET hostSocket, SOCKET clientSocket, mutex_t lock);

	void remove(SOCKET hostSocket);
};


#endif
