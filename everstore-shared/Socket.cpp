#include "Socket.h"
#include "Properties.h"
#include <stdexcept>

bool is_little_endian() {
	int num = 1;
	return (*(char*) &num == 1);
}

SharableSocket::SharableSocket(SOCKET socket, uint32_t bufferSize)
		: mSocket(socket), mBufferSize(bufferSize) {
}

SharableSocket::~SharableSocket() {
	if (mSocket != 0) {
		close(mSocket);
		mSocket = 0;
	}
}

SharableSocket* SharableSocket::accept() {
	const SOCKET s = ::accept(mSocket, NULL, 0);
	if (s == INVALID_SOCKET) {
		return nullptr;
	}

	if (!blocking(s)) {
		close(s);
		throw std::runtime_error("Failed to set socket into blocking mode");
	}

	if (!nodelay(s)) {
		close(s);
		throw std::runtime_error("Failed to set socket into immediate mode");
	}

	if (!bufferSize(s, mBufferSize)) {
		close(s);
		throw std::runtime_error("Failed to set buffer size used by the socket library");
	}

	return new SharableSocket(s, mBufferSize);
}

uint32_t SharableSocket::receive(char* bytes, uint32_t size) {
	uint32_t recvd = 0;
	while (recvd != size) {
		int32_t t = (int32_t) recv(mSocket, bytes, size - recvd, 0);
		if (t <= 0) return recvd;
		recvd += (uint32_t) t;
	}
	return recvd;
}

uint32_t SharableSocket::send(const char* bytes, uint32_t size) {
	uint32_t sent = 0;
	while (sent != size) {
		int32_t t = (int32_t) ::send(mSocket, bytes, size - sent, 0);
		if (t <= 0) return sent;
		sent += (uint32_t) t;
	}
	return sent;
}

void SharableSocket::init() {
#ifdef ES_WINDOWS
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		throw std::runtime_error("Failed to initialize socket library");
	}
#elif ES_GCC
	// No need to do anything here!
#endif
}

void SharableSocket::cleanup() {
#ifdef ES_WINDOWS
	WSACleanup();
#elif ES_GCC
	// No need to do anything here!
#endif
}

SharableSocket* SharableSocket::create(const Properties& p) {
	// Create a socket
	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET) {
		return nullptr;
	}

	if (!blocking(s)) {
		close(s);
		throw std::runtime_error("Failed to set socket into blocking mode");
	}

	if (!nodelay(s)) {
		close(s);
		throw std::runtime_error("Failed to set socket into immediate mode");
	}

	if (!bufferSize(s, p.maxDataSendSize)) {
		close(s);
		throw std::runtime_error("Failed to set buffer size used by the socket library");
	}

	SharableSocket* const socket = new SharableSocket(s, p.maxDataSendSize);
	return socket;
}

bool SharableSocket::blocking(SOCKET s) {
#ifdef ES_WINDOWS
	unsigned long param = 0;
	return ioctlsocket(s, FIONBIO, &param) == 0;
#elif ES_GCC
	unsigned long param = 0;
	if (ioctl(s, FIONBIO, &param) == -1)
		return false;
	return true;
#endif
}

void SharableSocket::close(SOCKET s) {
#if defined(ES_WINDOWS)
	closesocket(s);
#else
	::close(s)
#endif
}

bool SharableSocket::nodelay(SOCKET socket) {
	int flag = 1;
	setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, (char*) &flag, sizeof(int));
	return true;
}

bool SharableSocket::bufferSize(SOCKET s, uint32_t sizeInBytes) {
	int flag = sizeInBytes;
	setsockopt(s, IPPROTO_TCP, SO_SNDBUF, (char*) &flag, sizeof(int));
	setsockopt(s, IPPROTO_TCP, SO_RCVBUF, (char*) &flag, sizeof(int));
	return true;
}

bool SharableSocket::setTimeout(SOCKET socket, uint32_t millis) {
#ifdef ES_WINDOWS
	DWORD timeout = millis;
	if (setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (const char*) &timeout, sizeof(timeout)) != 0)
		return false;
	return setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, (const char*) &timeout, sizeof(timeout)) == 0;
#elif ES_GCC
	struct timeval timeout;
	timeout.tv_sec = millis / 1000;
	timeout.tv_usec = 0;

	if (setsockopt (socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) != 0)
		return false;
	return setsockopt (socket, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) != 0;
#endif
}
