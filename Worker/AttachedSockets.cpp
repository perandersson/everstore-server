#include "AttachedSockets.h"

AttachedConnection gAttachedSocket = {nullptr, nullptr};

AttachedSockets::AttachedSockets() {

}

AttachedSockets::~AttachedSockets() {
	for (auto& pair : mSockets) {
		delete pair.second->socket;
		delete pair.second->lock;
	}
}

AttachedConnection* AttachedSockets::get(OsSocket::Ref socketRef) {
	auto it = mSockets.find(socketRef);
	if (it == mSockets.end()) {
		return &gAttachedSocket;
	}
	return it->second;
}

void AttachedSockets::add(OsSocket::Ref socketFromHost, Socket* clientSocket, Mutex* lock) {
	auto a = new AttachedConnection();
	a->socket = clientSocket;
	a->lock = lock;
	mSockets.insert(make_pair(socketFromHost, a));
}

void AttachedSockets::remove(OsSocket::Ref socketFromHost) {
	auto it = mSockets.find(socketFromHost);
	if (it != mSockets.end()) {
		delete it->second->socket;
		delete it->second->lock;
		mSockets.erase(it);
	}
}

void AttachedSockets::clear() {
	for (auto& pair : mSockets) {
		delete pair.second->socket;
		delete pair.second->lock;
	}
	mSockets.clear();
}