//
// Copyright (c) 2019 West Coast Code AB. All rights reserved.
//

#include "Socket.hpp"
#include "../Log/Log.hpp"
#include "../Process/Process.hpp"

Socket::Socket(OsSocket::Ref socket, uint32_t bufferSize)
		: mSocket{socket}, mBufferSize(bufferSize) {
	Log::Write(Log::Info, "Socket(%p) | Socket is now created as SOCKET(%d)", this, socket);
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
		Log::Write(Log::Error, "Socket(%p) | Cannot accept a connection on an invalid socket", this);
		return nullptr;
	}

	OsSocket::Ref s = accept(mSocket.socket, nullptr, nullptr);
	if (s == OsSocket::Invalid) {
		Log::Write(Log::Warn, "Socket(%p) | Failed to accept socket. The socket might've been closed or timed out",
		           this);
		return nullptr;
	}

	auto const newSocket = new Socket(s, mBufferSize);

	// Make sure that the socket is in blocking mode
	ESErrorCode error = newSocket->SetBlocking();
	if (isError(error)) {
		Log::Write(Log::Warn, "Socket(%p) | Could not set new socket in blocking mode. %s (%d)", this,
		           parseErrorCode(error), error);
		delete newSocket;
		return nullptr;
	}

	// Do not save up for bytes until send.
	// TODO: This should be depending on how much traffic we are processing. If the server is constantly being under load then this should be disabled
	error = newSocket->SetNoDelay();
	if (isError(error)) {
		Log::Write(Log::Warn, "Socket(%p) | Could not set new socket in no-delay mode. %s (%d)", this,
		           parseErrorCode(error), error);
		delete newSocket;
		return nullptr;
	}

	// Set the buffer size
	error = newSocket->SetBufferSize(mBufferSize);
	if (isError(error)) {
		Log::Write(Log::Warn, "Socket(%p) | Could not set new socket's buffer size too %d. %s (%d)", this, mBufferSize,
		           parseErrorCode(error), error);
		delete newSocket;
		return nullptr;
	}

	return newSocket;
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
		if (t <= 0)
			return totalSend;
		totalSend += t;
	}
	return totalSend;
}

ESErrorCode Socket::ShareWithProcess(Process* process) {
	if (IsDestroyed()) {
		return ESERR_SCCKET_DESTROYED;
	}

	if (process == nullptr) {
		return ESERR_INVALID_ARGUMENT;
	}

	return OsSocket::ShareWithProcess(&mSocket, process->GetHandle());
}

Socket* Socket::CreateBlocking(uint32_t bufferSizeInBytes) {
	Log::Write(Log::Info, "Socket | Creating blocking socket");

	OsSocket handle;
	handle.socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (handle.socket == OsSocket::Invalid) {
		Log::Write(Log::Error, "Socket | Failed to create a new socket");
		return nullptr;
	}

	// Make sure that the socket is in blocking mode
	ESErrorCode error = OsSocket::SetBlocking(&handle);
	if (isError(error)) {
		Log::Write(Log::Error, "Socket | Failed to enable blocking mode on socket. %s (%d)", parseErrorCode(error),
		           error);
		OsSocket::Close(&handle);
		return nullptr;
	}

	// Do not save up for bytes until send
	error = OsSocket::SetNoDelay(&handle);
	if (isError(error)) {
		Log::Write(Log::Error, "Socket | Failed to disable delay mode on socket. %s (%d)", parseErrorCode(error),
		           error);
		OsSocket::Close(&handle);
		return nullptr;
	}

	// Set the buffer size
	error = OsSocket::SetBufferSize(&handle, bufferSizeInBytes);
	if (isError(error)) {
		Log::Write(Log::Error, "Socket | Failed to set the socket's buffer size to %d. %s (%d)", bufferSizeInBytes,
		           parseErrorCode(error), error);
		OsSocket::Close(&handle);
		return nullptr;
	}

	return new Socket(handle.socket, bufferSizeInBytes);
}

Socket* Socket::LoadFromProcess(Process* process, uint32_t bufferSizeInBytes) {
	if (process == nullptr) {
		Log::Write(Log::Error, "Socket | Cannot create a socket from an invalid process");
		return nullptr;
	}

	OsSocket handle;
	ESErrorCode error = OsSocket::LoadFromProcess(&handle, process->GetHandle());
	if (isError(error)) {
		Log::Write(Log::Error, "Socket | Failed to load socket from process. %s (%d)", parseErrorCode(error), error);
		return nullptr;
	}

	return new Socket(handle.socket, bufferSizeInBytes);
}
