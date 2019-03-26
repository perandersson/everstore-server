#ifndef _EVERSTORE_ATTACHED_SOCKETS_H_
#define _EVERSTORE_ATTACHED_SOCKETS_H_

#include "../Shared/everstore.h"

struct AttachedConnection
{
	SOCKET socket;
	mutex_t lock;
};

class AttachedSockets
{
public:
	AttachedSockets();

	~AttachedSockets();

	AttachedConnection* get(SOCKET s);

	void add(SOCKET hostSocket, SOCKET clientSocket, mutex_t lock);

	void remove(SOCKET hostSocket);

	void clear();

private:
	unordered_map<SOCKET, AttachedConnection*> mSockets;
};


#endif
