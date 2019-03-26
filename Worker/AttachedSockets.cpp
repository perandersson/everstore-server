#include "AttachedSockets.h"

AttachedConnection gAttachedSocket = { INVALID_SOCKET, INVALID_LOCK };

AttachedSockets::AttachedSockets() {

}

AttachedSockets::~AttachedSockets() {
	AttachedSockets& i = *this;
	for (unordered_map<SOCKET, AttachedConnection*>::value_type attachedSocket : i) {
		socket_close(attachedSocket.second->socket);
		mutex_destroy(attachedSocket.second->lock);
		delete attachedSocket.second;
	}
}

AttachedConnection* AttachedSockets::get(SOCKET s) {
	auto it = find(s);
	if (it == end()) return &gAttachedSocket;
	return it->second;
}

void AttachedSockets::add(SOCKET hostSocket, SOCKET clientSocket, mutex_t lock) {
	auto a = new AttachedConnection();
	a->socket = clientSocket;
	a->lock = lock;
	insert(make_pair(hostSocket, a));
}

void AttachedSockets::remove(SOCKET hostSocket) {
	auto it = find(hostSocket);
	if (it != end()) {
		socket_close(it->second->socket);
		mutex_destroy(it->second->lock);
		erase(it);
	}
}
