#ifndef _ES_SOCKET_H_
#define _ES_SOCKET_H_

#include <cinttypes>

#ifdef WIN32
#include "win32/Win32Socket.h"
#else
#include "gcc/GCCSocket.h"
#endif

// Initialize 
ESErrorCode socket_init();

// Cleanup the socket library
void socket_cleanup();

// Make the supplied socket blocking
ESErrorCode socket_setblocking(SOCKET socket);

// Set socket timeout
ESErrorCode socket_settimeout(SOCKET socket, uint32_t millis);

// Set the socket property to NO-DELAY
ESErrorCode socket_nodelay(SOCKET socket);

// Set send- and receive buffer size
ESErrorCode socket_setbufsize(SOCKET socket, uint32_t sizeInBytes);

// Create a new blocking socket
SOCKET socket_create_blocking(uint32_t maxBufferSize);

// Accept an incomming blocking socket
SOCKET socket_accept_blocking(SOCKET serverSocket, uint32_t maxBufferSize);

// Receive all bytes from the socket 
uint32_t socket_recvall(SOCKET socket, char* bytes, uint32_t size);

template<uint32_t max>
uint32_t socket_recvstring(SOCKET socket, _OUT string* s, uint32_t length) {
	// Clamp length
	length = length >= max ? max - 1 : length;

	// Read the characters
	char temp[max] = { 0 };
	uint32_t recv = socket_recvall(socket, temp, length);
	if (recv == 0) return 0;

	// Set and return the string
	*s = string(temp);
	return recv;
}

template<typename T>
uint32_t socket_recvall(SOCKET socket, T* object) {
	return socket_recvall(socket, (char*)object, (uint32_t)sizeof(T));
}

// Send all bytes on the socket
uint32_t socket_sendall(SOCKET socket, const char* bytes, uint32_t size);

template<typename T>
uint32_t socket_sendall(SOCKET socket, const T* object) {
	return socket_sendall(socket, (const char*)object, (uint32_t)sizeof(T));
}

// Check if this machine is handles integers in little-endian mode
bool is_little_endian();

#endif
