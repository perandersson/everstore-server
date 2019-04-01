//
// Copyright (c) 2019 West Coast Code AB. All rights reserved.
//

#include "Socket.hpp"
#include "../Log/Log.hpp"

Socket::Socket(OsSocket::Ref socket, uint32_t bufferSize)
		: mSocket{socket}, mBufferSize(bufferSize) {
}

Socket::~Socket() {
	Socket::Destroy();
}

bool Socket::Initialize() {
#ifdef _WIN32
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		return false;
	}
#endif
	return true;
}

void Socket::Shutdown() {
#ifdef _WIN32
	WSACleanup();
#endif
}

ESErrorCode Socket::Destroy() {
	// Ignore if the socket is already destroyed
	if (IsDestroyed()) {
		return ESERR_NO_ERROR;
	}
	return Close(mSocket.socket);
}

ESErrorCode Socket::Listen(Port port, uint32_t maxConnections) {
	if (IsDestroyed()) {
		return ESERR_SCCKET_DESTROYED;
	}

	// TODO: Replace with the network interface we want to listen for connections from
	sockaddr_in addr;
	memset(&addr, 0, sizeof(sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port.value);
	if (::bind(mSocket.socket, (struct sockaddr*) &addr, sizeof addr) == -1) {
		Destroy();
		return ESERR_SOCKET_BIND;
	}

	if (::listen(mSocket.socket, maxConnections) == -1) {
		Destroy();
		return ESERR_SOCKET_LISTEN;
	}

	return ESERR_NO_ERROR;
}

Socket* Socket::AcceptBlocking() {
	if (IsDestroyed()) {
		return nullptr;
	}

	OsSocket::Ref s = accept(mSocket.socket, nullptr, nullptr);
	if (s == OsSocket::Invalid) {
		return nullptr;
	}

	// Make sure that the socket is in blocking mode
	ESErrorCode error = SetBlocking(s);
	if (isError(error)) {
		Close(s);
		return nullptr;
	}

	// Do not save up for bytes until send
	error = SetNoDelay(s);
	if (isError(error)) {
		Close(s);
		return nullptr;
	}

	// Set the buffer size
	error = SetBufferSize(s, mBufferSize);
	if (isError(error)) {
		Close(s);
		return nullptr;
	}

	return new Socket(s, mBufferSize);
}

int32_t Socket::ReceiveAll(char* buffer, uint32_t size) {
	if (IsDestroyed()) {
		return -1;
	}

	int32_t totalRecv = 0;
	while (totalRecv != size) {
		const auto t = recv(mSocket.socket, buffer, size - totalRecv, 0);
		if (t <= 0)
			return totalRecv;
		totalRecv += t;
	}
	return totalRecv;
}

int32_t Socket::SendAll(const char* bytes, uint32_t size) {
	if (IsDestroyed()) {
		return -1;
	}

	int32_t totalSend = 0;
	while (totalSend != size) {
		const auto t = send(mSocket.socket, bytes, size - totalSend, 0);
		if (t <= 0) return totalSend;
		totalSend += t;
	}
	return totalSend;
}

Socket* Socket::CreateBlocking(uint32_t bufferSizeInBytes) {
	OsSocket::Ref s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == OsSocket::Invalid) {
		Log::Write(Log::Error, "Failed to create a new socket");
		return nullptr;
	}

	// Make sure that the socket is in blocking mode
	ESErrorCode error = SetBlocking(s);
	if (isError(error)) {
		Log::Write(Log::Error, "Failed to enable blocking mode on socket. Reason: %s (%d)", parseErrorCode(error),
		           error);
		Close(s);
		return nullptr;
	}

	// Do not save up for bytes until send
	error = SetNoDelay(s);
	if (isError(error)) {
		Log::Write(Log::Error, "Failed to disable delay mode on socket. Reason: %s (%d)", parseErrorCode(error),
		           error);
		Close(s);
		return nullptr;
	}

	// Set the buffer size
	error = SetBufferSize(s, bufferSizeInBytes);
	if (isError(error)) {
		Log::Write(Log::Error, "Failed to set the socket's buffer size to %d. Reason: %s (%d)", bufferSizeInBytes,
		           parseErrorCode(error), error);
		Close(s);
		return nullptr;
	}

	return new Socket(s, bufferSizeInBytes);
}

ESErrorCode Socket::SetBufferSize(OsSocket::Ref socket, uint32_t sizeInBytes) {
	if (socket == OsSocket::Invalid) {
		return ESERR_SCCKET_DESTROYED;
	}

#ifdef _WIN32
	int flag = sizeInBytes;
	if (setsockopt(socket, IPPROTO_TCP, SO_SNDBUF, (const char*) &flag, sizeof(int)) != 0)
		return ESERR_SOCKET_CONFIGURE;
	if (setsockopt(socket, IPPROTO_TCP, SO_RCVBUF, (const char*) &flag, sizeof(int)) != 0)
		return ESERR_SOCKET_CONFIGURE;
	return ESERR_NO_ERROR;
#else
	int flag = sizeInBytes;
	if (setsockopt(mSocket, IPPROTO_TCP, SO_SNDBUF, (char*) &flag, sizeof(int)) != 0)
		return ESERR_SOCKET_CONFIGURE;
	if (setsockopt(mSocket, IPPROTO_TCP, SO_RCVBUF, (char*) &flag, sizeof(int)) != 0)
		return ESERR_SOCKET_CONFIGURE;
	return ESERR_NO_ERROR;
#endif
}

ESErrorCode Socket::SetBlocking(OsSocket::Ref socket) {
	if (socket == OsSocket::Invalid) {
		return ESERR_SCCKET_DESTROYED;
	}

#ifdef _WIN32
	unsigned long param = 0;
	if (ioctlsocket(socket, FIONBIO, &param) == 0)
		return ESERR_NO_ERROR;
#else
	unsigned long param = 0;
	if (ioctlsocket(socket, FIONBIO, &param) == 0)
		return ESERR_NO_ERROR;
#endif

	return ESERR_SOCKET_CONFIGURE;
}

ESErrorCode Socket::SetTimeout(OsSocket::Ref socket, uint32_t millis) {
	if (socket == OsSocket::Invalid) {
		return ESERR_SCCKET_DESTROYED;
	}

#ifdef _WIN32
	const DWORD timeout = millis;

	if (setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (const char*) &timeout, sizeof(timeout)) != 0)
		return ESERR_SOCKET_CONFIGURE;

	if (setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, (const char*) &timeout, sizeof(timeout)) != 0)
		return ESERR_SOCKET_CONFIGURE;

	return ESERR_NO_ERROR;
#else
	struct timeval timeout;
	timeout.tv_sec = millis / 1000;
	timeout.tv_usec = 0;

	if (setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (char*) &timeout, sizeof(timeout)) != 0)
		return ESERR_SOCKET_CONFIGURE;

	if (setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, (char*) &timeout, sizeof(timeout)) != 0)
		return ESERR_SOCKET_CONFIGURE;

	return ESERR_NO_ERROR;
#endif
}

ESErrorCode Socket::SetNoDelay(OsSocket::Ref socket) {
	if (socket == OsSocket::Invalid) {
		return ESERR_SCCKET_DESTROYED;
	}

#ifdef _WIN32
	int flag = 1;
	if (setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, (char*) &flag, sizeof(int)) != 0)
		return ESERR_SOCKET_CONFIGURE;
	return ESERR_NO_ERROR;
#else
	int flag = 1;
	if (setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, (char*) &flag, sizeof(int)) != 0)
		return ESERR_SOCKET_CONFIGURE;
	return ESERR_NO_ERROR;
#endif
}

ESErrorCode Socket::Close(OsSocket::Ref socket) {
#ifdef _WIN32
	const auto ret = closesocket(socket);
	if (ret != 0) {
		return ESERR_SOCKET;
	}
#else
	::close(socket);
#endif
	return ESERR_NO_ERROR;
}
