#include "Win32Socket.h"
#include "../Socket.h"
#include "process.h"
#include "../Socket/Win32/Win32Socket.hpp"


ESErrorCode socket_setblocking(SOCKET socket) {
	unsigned long param = 0;
	if (ioctlsocket(socket, FIONBIO, &param) == 0)
		return ESERR_NO_ERROR;
	return ESERR_SOCKET_CONFIGURE;
}

ESErrorCode socket_settimeout(SOCKET socket, uint32_t millis) {
	DWORD timeout = millis;
	if (setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout)) == 0)
		return ESERR_NO_ERROR;
	if (setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout)) == 0)
		return ESERR_NO_ERROR;
	return ESERR_SOCKET_CONFIGURE;
}

ESErrorCode socket_nodelay(SOCKET socket) {

	int flag = 1;
	setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(int));

	return ESERR_NO_ERROR;
}

ESErrorCode socket_setbufsize(SOCKET socket, uint32_t sizeInBytes) {
	int flag = sizeInBytes;
	setsockopt(socket, IPPROTO_TCP, SO_SNDBUF, (char*)&flag, sizeof(int));
	setsockopt(socket, IPPROTO_TCP, SO_RCVBUF, (char*)&flag, sizeof(int));
	return ESERR_NO_ERROR;
}
