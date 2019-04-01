#include <sys/un.h>
#include <netinet/tcp.h>
#include "GCCSocket.h"
#include "../Process.h"

ESErrorCode socket_setblocking(SOCKET socket) {
	unsigned long param = 0;
	return ioctl(socket, FIONBIO, &param) == -1 ? ESERR_SOCKET_CONFIGURE : ESERR_NO_ERROR;
}

ESErrorCode socket_settimeout(SOCKET socket, uint32_t millis) {
	struct timeval timeout;
	timeout.tv_sec = millis / 1000;
	timeout.tv_usec = 0;

	if (setsockopt (socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
		return ESERR_SOCKET_CONFIGURE;

	if (setsockopt (socket, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
		return ESERR_SOCKET_CONFIGURE;

	return ESERR_NO_ERROR;
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
