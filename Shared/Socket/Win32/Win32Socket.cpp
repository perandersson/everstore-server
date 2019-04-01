//
// Copyright (c) 2019 West Coast Code AB. All rights reserved.
//

#include "Win32Socket.hpp"
#include "../../Process/Win32/Win32Process.hpp"

ESErrorCode OsSocket::ShareWithProcess(OsSocket* socket, OsProcess* process) {
	if (IsInvalid(socket)) {
		return ESERR_SCCKET_DESTROYED;
	}

	// Copy the handle so that we can share it with other processes
	WSAPROTOCOL_INFO info;
	if (WSADuplicateSocket(socket->socket, process->processInfo.dwProcessId, &info) == SOCKET_ERROR) {
		return ESSER_PROCESS_SHARE_SOCKET;
	}

	// Send the duplicated socket information to the child-process
	const auto writtenBytes = OsProcess::Write(process, reinterpret_cast<const char*>(&info), sizeof(info));
	if (writtenBytes != sizeof(info)) {
		return ESERR_PIPE_WRITE;
	}

	return ESERR_NO_ERROR;
}

ESErrorCode OsSocket::LoadFromProcess(OsSocket* socket, OsProcess* process) {
	// Read the spared socket protocol information sent by the parent process
	WSAPROTOCOL_INFO info;
	const auto readBytes = OsProcess::Read(process, reinterpret_cast<char*>(&info), sizeof info);
	if (readBytes != sizeof info) {
		return ESERR_PIPE_READ;
	}

	// Create a socket based on the shared socket information
	socket->socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, &info, 0, 0);
	if (socket->socket == INVALID_SOCKET) {
		return ESERR_PROCESS_ATTACH_SHARED_SOCKET;
	}

	// Make sure to block thread while waiting for incoming data
	ESErrorCode err = SetBlocking(socket);
	if (isError(err)) {
		Close(socket);
		return err;
	}

	return ESERR_NO_ERROR;
}

ESErrorCode OsSocket::SetBufferSize(OsSocket* socket, uint32_t sizeInBytes) {
	if (IsInvalid(socket)) {
		return ESERR_SCCKET_DESTROYED;
	}
	DWORD flag = sizeInBytes;
	auto error = WSAGetLastError();
	if (setsockopt(socket->socket, SOL_SOCKET, SO_SNDBUF, (const char*)&flag, sizeof(int)) != 0) {
		error = WSAGetLastError();
		return ESERR_SOCKET_CONFIGURE;
	}
	if (setsockopt(socket->socket, SOL_SOCKET, SO_RCVBUF, (const char*) &flag, sizeof(int)) != 0)
		return ESERR_SOCKET_CONFIGURE;
	return ESERR_NO_ERROR;
}

ESErrorCode OsSocket::SetBlocking(OsSocket* socket) {
	if (IsInvalid(socket)) {
		return ESERR_SCCKET_DESTROYED;
	}
	unsigned long param = 0;
	if (ioctlsocket(socket->socket, FIONBIO, &param) == 0)
		return ESERR_NO_ERROR;
	return ESERR_SOCKET_CONFIGURE;
}

ESErrorCode OsSocket::SetTimeout(OsSocket* socket, uint32_t millis) {
	if (IsInvalid(socket)) {
		return ESERR_SCCKET_DESTROYED;
	}
	const DWORD timeout = millis;
	if (setsockopt(socket->socket, SOL_SOCKET, SO_RCVTIMEO, (const char*) &timeout, sizeof(timeout)) != 0)
		return ESERR_SOCKET_CONFIGURE;
	if (setsockopt(socket->socket, SOL_SOCKET, SO_SNDTIMEO, (const char*) &timeout, sizeof(timeout)) != 0)
		return ESERR_SOCKET_CONFIGURE;
	return ESERR_NO_ERROR;
}

ESErrorCode OsSocket::SetNoDelay(OsSocket* socket) {
	if (IsInvalid(socket)) {
		return ESERR_SCCKET_DESTROYED;
	}

	int flag = 1;
	if (setsockopt(socket->socket, IPPROTO_TCP, TCP_NODELAY, (char*) &flag, sizeof(int)) != 0)
		return ESERR_SOCKET_CONFIGURE;
	return ESERR_NO_ERROR;
}

ESErrorCode OsSocket::Close(OsSocket* socket) {
	if (IsInvalid(socket)) {
		return ESERR_SCCKET_DESTROYED;
	}

	const auto ret = closesocket(socket->socket);
	socket->socket = Invalid;
	if (ret != 0) {
		return ESERR_SOCKET;
	}
	return ESERR_NO_ERROR;
}
