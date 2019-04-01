//
// Copyright (c) 2019 West Coast Code AB. All rights reserved.
//

#ifndef EVERSTORE_WIN32SOCKET_HPP
#define EVERSTORE_WIN32SOCKET_HPP

#include <winsock2.h>
#include <windows.h>

struct OsSocket
{
	typedef SOCKET Ref;
	static constexpr auto Invalid = INVALID_SOCKET;

	Ref socket;

	static bool IsInvalid(const OsSocket* s) { return s->socket == INVALID_SOCKET; }
};

#endif //EVERSTORE_WIN32SOCKET_HPP
