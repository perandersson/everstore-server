#ifndef _EVERSTORE_WIN32_SOCKET_H_
#define _EVERSTORE_WIN32_SOCKET_H_

#include "../ESErrorCodes.h"

#include <winsock2.h>
#include <sys/types.h>
#include <ws2tcpip.h>

//typedef int socklen_t;
//typedef long ssize_t;

// Macro for closing the socket
#define socket_close closesocket

struct socket_t {
	WSAPROTOCOL_INFO info;
};


#endif
