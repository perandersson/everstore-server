#include "AttachedSockets.h"

AttachedConnection gAttachedSocket = {INVALID_SOCKET, INVALID_LOCK};

AttachedSockets::AttachedSockets() {

}

AttachedSockets::~AttachedSockets() {
	for (auto& attachedSocket : mSockets) {
		socket_close(attachedSocket.second->socket);
		mutex_destroy(attachedSocket.second->lock);
		delete attachedSocket.second;
	}
}

AttachedConnection* AttachedSockets::get(SOCKET s) {
	auto it = mSockets.find(s);
	if (it == mSockets.end()) return &gAttachedSocket;
	return it->second;
}

void AttachedSockets::add(SOCKET hostSocket, SOCKET clientSocket, mutex_t lock) {
	auto a = new AttachedConnection();
	a->socket = clientSocket;
	a->lock = lock;
	mSockets.insert(make_pair(hostSocket, a));
}

void AttachedSockets::remove(SOCKET hostSocket) {
	auto it = mSockets.find(hostSocket);
	if (it != mSockets.end()) {
		socket_close(it->second->socket);
		mutex_destroy(it->second->lock);
		mSockets.erase(it);
	}
}

void AttachedSockets::clear() {
	mSockets.clear();
}