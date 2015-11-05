#include "Socket.h"

SOCKET socket_create_blocking() {
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
	socket_setbufsize(s, DEFAULT_MAX_DATA_SEND_SIZE);
	return s;
}

SOCKET socket_accept_blocking(SOCKET serverSocket) {
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
	socket_setbufsize(socket, DEFAULT_MAX_DATA_SEND_SIZE);
	return socket;
}

uint32_t socket_recvall(SOCKET socket, char* bytes, uint32_t size) {
	uint32_t recvd = 0;
	while (recvd != size) {
		int32_t t = (int32_t)recv(socket, bytes, size - recvd, 0);
		if (t <= 0) return recvd;
		recvd += (uint32_t)t;
	}
	return recvd;
}

uint32_t socket_sendall(SOCKET socket, const char* bytes, uint32_t size) {
	uint32_t sent = 0;
	while (sent != size) {
		int32_t t = (int32_t)send(socket, bytes, size - sent, 0);
		if (t <= 0) return sent;
		sent += (uint32_t)t;
	}
	return sent;
}

bool is_little_endian() {
	int num = 1;
	return (*(char *)&num == 1);
}