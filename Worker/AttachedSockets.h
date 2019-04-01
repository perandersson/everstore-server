#ifndef _EVERSTORE_ATTACHED_SOCKETS_H_
#define _EVERSTORE_ATTACHED_SOCKETS_H_

#include "../Shared/everstore.h"

struct AttachedConnection
{
	Socket* socket;
	Mutex* lock;
};

class AttachedSockets
{
public:
	AttachedSockets();

	~AttachedSockets();

	AttachedConnection* get(OsSocket::Ref socketRef);

	void add(OsSocket::Ref socketFromHost, Socket* clientSocket, Mutex* lock);

	void remove(OsSocket::Ref socketFromHost);

	void clear();

private:
	unordered_map<OsSocket::Ref, AttachedConnection*> mSockets;
};


#endif
