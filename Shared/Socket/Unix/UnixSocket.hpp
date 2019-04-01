//
// Copyright (c) 2019 West Coast Code AB. All rights reserved.
//

#ifndef EVERSTORE_UNIXSOCKET_HPP
#define EVERSTORE_UNIXSOCKET_HPP

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <sys/un.h>
#include "../../ESErrorCodes.h"

struct OsProcess;

struct OsSocket
{
	typedef int Ref;
	static constexpr auto Invalid = -1;

	Ref socket;

	static ESErrorCode ShareWithProcess(OsSocket* socket, OsProcess* process);

	static ESErrorCode LoadFromProcess(OsSocket* socket, OsProcess* process);

	static ESErrorCode SetBufferSize(OsSocket* socket, uint32_t sizeInBytes);

	static ESErrorCode SetBlocking(OsSocket* socket);

	static ESErrorCode SetTimeout(OsSocket* socket, uint32_t millis);

	static ESErrorCode SetNoDelay(OsSocket* socket);

	static ESErrorCode Close(OsSocket* socket);

	static bool IsInvalid(const OsSocket* s) { return s->socket == Invalid; }
};


#endif //EVERSTORE_UNIXSOCKET_HPP
