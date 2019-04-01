//
// Copyright (c) 2019 West Coast Code AB. All rights reserved.
//

#ifndef EVERSTORE_WIN32SOCKET_HPP
#define EVERSTORE_WIN32SOCKET_HPP

#include <winsock2.h>
#include <windows.h>
#include "../../ESErrorCodes.h"

struct OsProcess;

struct OsSocket
{
	typedef SOCKET Ref;
	static constexpr auto Invalid = INVALID_SOCKET;

	Ref socket;

	static ESErrorCode ShareWithProcess(OsSocket* socket, OsProcess* process);

	static ESErrorCode LoadFromProcess(OsSocket* socket, OsProcess* process);

	static ESErrorCode SetBufferSize(OsSocket* socket, uint32_t sizeInBytes);

	static ESErrorCode SetBlocking(OsSocket* socket);

	static ESErrorCode SetTimeout(OsSocket* socket, uint32_t millis);

	static ESErrorCode SetNoDelay(OsSocket* socket);

	static ESErrorCode Close(OsSocket* socket);

	static bool IsInvalid(const OsSocket* s) { return s->socket == INVALID_SOCKET; }
};

#endif //EVERSTORE_WIN32SOCKET_HPP
