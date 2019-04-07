//
// Copyright (c) 2019 West Coast Code AB. All rights reserved.
//

#include "UnixSocket.hpp"
#include "../../Process/Unix/UnixProcess.hpp"
#include "../../Log/Log.hpp"
#include <sys/un.h>
#include <netinet/tcp.h>

ESErrorCode OsSocket::ShareWithProcess(OsSocket* socket, OsProcess* process) {
	if (IsInvalid(socket)) {
		return ESERR_SCCKET_DESTROYED;
	}

	if (OsProcess::IsInvalid(process)) {
		return ESERR_PROCESS_DESTROYED;
	}

	// Prepare memory structures used for sharing a socket with another process
	msghdr message;
	iovec iov[1];
	cmsghdr* control_message = nullptr;
	char ctrl_buf[CMSG_SPACE(sizeof(int))];
	char data[1];

	memset(&message, 0, sizeof(struct msghdr));
	memset(ctrl_buf, 0, CMSG_SPACE(sizeof(int)));

	// We are passing at least one byte of data so that recvmsg() will not return 0
	data[0] = ' ';
	iov[0].iov_base = data;
	iov[0].iov_len = sizeof(data);

	message.msg_name = nullptr;
	message.msg_namelen = 0;
	message.msg_iov = iov;
	message.msg_iovlen = 1;
	message.msg_controllen = CMSG_SPACE(sizeof(int));
	message.msg_control = ctrl_buf;

	control_message = CMSG_FIRSTHDR(&message);
	control_message->cmsg_level = SOL_SOCKET;
	control_message->cmsg_type = SCM_RIGHTS;
	control_message->cmsg_len = CMSG_LEN(sizeof(int));

	*((int*) CMSG_DATA(control_message)) = socket->socket;

	// Send a message containing the socket over the memory pipe. Messages and data are different on unix'ish platforms.
	// Message are sent with "sendmsg" and data are sent with "send".
	ssize_t len = sendmsg(process->unixSocket, &message, 0);
	if (len <= 0) {
		Log::Write(Log::Error, "OsSocket | Failed to send Socket(%d) to process UnixSocket(%d)", socket->socket,
		           process->unixSocket);
		return ESSER_PROCESS_SHARE_SOCKET;
	}
	return ESERR_NO_ERROR;
}

ESErrorCode OsSocket::LoadFromProcess(OsSocket* socket, OsProcess* process) {
	if (socket == nullptr) {
		return ESERR_INVALID_ARGUMENT;
	}

	if (OsProcess::IsInvalid(process)) {
		return ESERR_PROCESS_DESTROYED;
	}

	socket->socket = Invalid;

	// Prepare the structure used to receive the socket
	msghdr message;
	iovec iov[1];
	cmsghdr* control_message = nullptr;
	char ctrl_buf[CMSG_SPACE(sizeof(int))];
	char data[1] = {' '};
	memset(&message, 0, sizeof(struct msghdr));
	memset(ctrl_buf, 0, CMSG_SPACE(sizeof(int)));

	// For the dummy data
	iov[0].iov_base = data;
	iov[0].iov_len = sizeof(data);

	message.msg_name = nullptr;
	message.msg_namelen = 0;
	message.msg_control = ctrl_buf;
	message.msg_controllen = CMSG_SPACE(sizeof(int));
	message.msg_iov = iov;
	message.msg_iovlen = 1;

	// Read the actual message sent over the pipe. The socket is part message
	if (recvmsg(process->unixSocket, &message, 0) <= 0) {
		Log::Write(Log::Debug, "OsSocket | Failed to read socket data from UnixSocket(%d)", process->unixSocket);
		return ESERR_PIPE_READ;
	}

	// Iterate through header to find if there is a file descriptor
	for (control_message = CMSG_FIRSTHDR(&message);
	     control_message != nullptr;
	     control_message = CMSG_NXTHDR(&message, control_message)) {
		if ((control_message->cmsg_level == SOL_SOCKET) &&
		    (control_message->cmsg_type == SCM_RIGHTS)) {
			socket->socket = *((int*) CMSG_DATA(control_message));
		}
	}

	if (socket->socket == Invalid) {
		return ESERR_SOCKET_NOT_ATTACHED;
	}
	return ESERR_NO_ERROR;
}

ESErrorCode OsSocket::SetBufferSize(OsSocket* socket, uint32_t sizeInBytes) {
	if (IsInvalid(socket)) {
		return ESERR_SCCKET_DESTROYED;
	}

	int flag = sizeInBytes;
	if (setsockopt(socket->socket, SOL_SOCKET, SO_SNDBUF, (const char*) &flag, sizeof(int)) != 0)
		return ESERR_SOCKET_CONFIGURE;
	if (setsockopt(socket->socket, SOL_SOCKET, SO_RCVBUF, (const char*) &flag, sizeof(int)) != 0)
		return ESERR_SOCKET_CONFIGURE;
	return ESERR_NO_ERROR;
}

ESErrorCode OsSocket::SetBlocking(OsSocket* socket) {
	if (IsInvalid(socket)) {
		return ESERR_SCCKET_DESTROYED;
	}

	unsigned long param = 0;
	if (ioctl(socket->socket, FIONBIO, &param) == -1)
		return ESERR_SOCKET_CONFIGURE;
	return ESERR_NO_ERROR;
}

ESErrorCode OsSocket::SetTimeout(OsSocket* socket, uint32_t millis) {
	if (IsInvalid(socket)) {
		return ESERR_SCCKET_DESTROYED;
	}

	struct timeval timeout;
	timeout.tv_sec = millis / 1000;
	timeout.tv_usec = 0;

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
	if (setsockopt(socket->socket, IPPROTO_TCP, TCP_NODELAY, (const char*) &flag, sizeof(int)) == -1)
		return ESERR_SOCKET_CONFIGURE;

	return ESERR_NO_ERROR;
}

ESErrorCode OsSocket::Close(OsSocket* socket) {
	if (socket->socket != Invalid) {
		::close(socket->socket);
		socket->socket = Invalid;
	}

	return ESERR_NO_ERROR;
}
