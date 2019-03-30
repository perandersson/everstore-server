#include "Socket.h"

SOCKET socket_create_blocking(uint32_t maxBufferSize) {
	// Create a socket
	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET) {
		return s;
	}

	// Make sure to block
	ESErrorCode err = socket_setblocking(s);
	if (isError(err)) {
		socket_close(s);
		return 0;
	}

	socket_nodelay(s);
	socket_setbufsize(s, maxBufferSize);
	return s;
}

SOCKET socket_accept_blocking(SOCKET serverSocket, uint32_t maxBufferSize) {
	SOCKET socket = ::accept(serverSocket, NULL, 0);
	if (socket == INVALID_SOCKET) {
		return INVALID_SOCKET;
	}

	// Make sure to block
	ESErrorCode err = socket_setblocking(socket);
	if (isError(err)) {
		socket_close(socket);
		return 0;
	}

	socket_nodelay(socket);
	socket_setbufsize(socket, maxBufferSize);
	return socket;
}

int32_t socket_recvall(SOCKET socket, char* bytes, int32_t size) {
	int32_t recvd = 0;
	while (recvd != size) {
		int32_t t = (int32_t) recv(socket, bytes, size - recvd, 0);
		if (t <= 0) return t;
		recvd += t;
	}
	return recvd;
}

int32_t socket_sendall(SOCKET socket, const char* bytes, int32_t size) {
	int32_t sent = 0;
	while (sent != size) {
		int32_t t = (int32_t) send(socket, bytes, size - sent, 0);
		if (t <= 0) return t;
		sent += t;
	}
	return sent;
}

bool is_little_endian() {
	int num = 1;
	return (*(char*) &num == 1);
}