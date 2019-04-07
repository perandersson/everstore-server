//
// Copyright (c) 2019 West Coast Code AB. All rights reserved.
//

#include "UnixProcess.hpp"
#include <sys/un.h>
#include <sys/socket.h>
#include "../../Socket/Unix/UnixSocket.hpp"
#include "../../Log/Log.hpp"

const string gPipeNamePrefix = "everstore_pipe_child";

int32_t UnixSocketReceiveAll(int unixSocket, char* buffer, uint32_t size) {
	if (unixSocket == OsProcess::InvalidPipe) {
		return -1;
	}

	int32_t totalRecv = 0;
	while (totalRecv != size) {
		const auto t = recv(unixSocket, buffer, size - totalRecv, 0);
		if (t <= 0)
			return totalRecv;
		totalRecv += t;
	}
	return totalRecv;
}

int32_t UnixSocketSendAll(int unixSocket, const char* bytes, uint32_t size) {
	if (unixSocket == OsProcess::InvalidPipe) {
		return -1;
	}

	int32_t totalSend = 0;
	while (totalSend != size) {
		const auto t = send(unixSocket, bytes, size - totalSend, 0);
		if (t <= 0)
			return totalSend;
		totalSend += t;
	}
	return totalSend;
}

ESErrorCode AcceptBlockingUnixSocket(const int listener, int* result, uint32_t bufferSize) {
	// Accept the incoming Unix Socket
	*result = OsProcess::InvalidPipe;
	OsSocket::Ref newSocket = accept(listener, nullptr, nullptr);
	if (newSocket == OsSocket::Invalid) {
		return ESERR_SOCKET_ACCEPT;
	}

	// Make sure that the socket is in blocking mode
	unsigned long param = 0;
	ioctl(newSocket, FIONBIO, &param);

	// Set the buffer size
	int flag = bufferSize;
	setsockopt(newSocket, SOL_SOCKET, SO_SNDBUF, (const char*) &flag, sizeof(int));
	setsockopt(newSocket, SOL_SOCKET, SO_RCVBUF, (const char*) &flag, sizeof(int));

	*result = newSocket;
	return ESERR_NO_ERROR;
}

ESErrorCode CreateUnixSocket(int* unixSocket, uint32_t bufferSize) {
	int sd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sd == OsProcess::InvalidPipe) {
		Log::Write(Log::Error, "OsProcess | Failed to create a new unix socket");
		return ESERR_SOCKET;
	}

	// Make sure that the socket is in blocking mode
	unsigned long param = 0;
	ioctl(sd, FIONBIO, &param);

	*unixSocket = sd;
	return ESERR_NO_ERROR;
}

ESErrorCode UnixCreatePipe(ProcessID id, int* unixSocket, int32_t bufferSize) {
	// Create a unix socket
	auto const error = CreateUnixSocket(unixSocket, bufferSize);
	if (isError(error)) {
		return error;
	}

	// Unlink any pre-existing unix sockets. This ensures that we remove any unix sockets that was not removed
	// properly. (can happen if the application crashes)
	const string name = gPipeNamePrefix + id.ToString();
	unlink(name.c_str());

	// Start listening for data and stuff over the unix socket
	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, name.c_str());
	if (::bind(*unixSocket, (struct sockaddr*) &(addr), sizeof(addr)) < 0) {
		::close(*unixSocket);
		Log::Write(Log::Error, "OsProcess | Could not bind %s with UnixSocket(%d)", name.c_str(), *unixSocket);
		return ESERR_PIPE_LISTEN;
	}

	if (::listen(*unixSocket, 1) < 0) {
		::close(*unixSocket);
		Log::Write(Log::Error, "OsProcess | Could not listen for incoming requests on UnixSocket(%d)", name.c_str(),
		           *unixSocket);
		return ESERR_PIPE_LISTEN;
	}

	return ESERR_NO_ERROR;
}

ESErrorCode OsProcess::Start(ProcessID id, const Path& command, const vector<string>& args, int32_t bufferSize,
                             OsProcess* result) {
	result->unixSocket = InvalidPipe;
	result->handle = InvalidProcess;
	result->running = false;

	// Start by opening the memory pipe
	OsSocket::Ref unixSocket;
	ESErrorCode err = UnixCreatePipe(id, &unixSocket, bufferSize);
	if (isError(err)) {
		return err;
	}

	// Merge the arguments and the command into a new array
	const char* argv[args.size() + 2];
	argv[0] = command.value.c_str();
	for (auto i = 0; i < args.size(); ++i) {
		argv[i + 1] = args[i].c_str();
	}
	argv[args.size() + 1] = nullptr;

	// Duplicate this process. Unix is special in the way that "fork()" will return 0 for the current process
	// and something else if we've allocated new memory for the new process.
	pid_t pid = fork();
	if (pid != 0) {
		result->handle = pid;
	} else {
		// Replace this forked process with the child-process executable file. The "execvp" will only return
		// if an error has occurred (i.e. the command is not found)
		const auto result = execvp(command.value.c_str(), (char**) argv);

		// If the server failed to replace the new/forked process with a new process then shut it down
		Log::Write(Log::Error, "OsProcess | Failed to start child process because %d", command.value.c_str(), result);
		abort();
	}

	// Connect to the pipe and discard the main unix socket for this process. We don't need it anymore. This is because
	// each unix socket is a 1 to 1 connection between the parent- and child process
	err = AcceptBlockingUnixSocket(unixSocket, &result->unixSocket, bufferSize);
	::close(unixSocket);
	if (isError(err)) {
		::close(result->unixSocket);
		return err;
	}
	result->running = true;
	return ESERR_NO_ERROR;
}

ESErrorCode OsProcess::Destroy(OsProcess* process) {
	// Kill the child-process if one exists
	if (process->handle != InvalidProcess) {
		killpg(process->handle, SIGKILL);
		// Wait for 30 seconds until we're closing the processes
		static constexpr auto TimeoutMillis = 30u * 1000u;
		WaitForClosed(process, TimeoutMillis);
		process->handle = InvalidProcess;
	}

	// Close the unix socket
	if (process->unixSocket != InvalidPipe) {
		::close(process->unixSocket);
		process->unixSocket = InvalidPipe;
	}

	process->running = false;
	return ESERR_NO_ERROR;
}

ESErrorCode OsProcess::WaitForClosed(OsProcess* process, uint32_t timeout) {
	// TODO: Using sleep for now, but try to implement this in a smarter way
	this_thread::sleep_for(chrono::microseconds(1000));
	return ESERR_NO_ERROR;
}

ESErrorCode OsProcess::ConnectToHost(ProcessID id, int32_t bufferSize, OsProcess* process) {
	process->unixSocket = InvalidPipe;
	process->handle = InvalidProcess;
	process->running = false;

	const auto name = gPipeNamePrefix + id.ToString();

	// Start by creating a unix socket
	int unixSocket;
	ESErrorCode error = CreateUnixSocket(&unixSocket, bufferSize);
	if (isError(error)) {
		return error;
	}

	// Then connect to it
	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, name.c_str());
	if (::connect(unixSocket, (struct sockaddr*) &(addr), sizeof(addr)) < 0) {
		::close(unixSocket);
		return ESERR_PIPE_CONNECT;
	}

	process->unixSocket = unixSocket;
	process->running = true;
	return ESERR_NO_ERROR;
}

int32_t OsProcess::Write(OsProcess* process, const char* bytes, uint32_t size) {
	if (IsInvalid(process)) {
		Log::Write(Log::Error, "OsProcess | Cannot write data on a closed process");
		return -1;
	}

	return UnixSocketSendAll(process->unixSocket, bytes, size);
}

int32_t OsProcess::Read(OsProcess* process, char* bytes, uint32_t size) {
	if (IsInvalid(process)) {
		Log::Write(Log::Debug2, "OsProcess | Cannot read data from a closed process");
		return -1;
	}

	return UnixSocketReceiveAll(process->unixSocket, bytes, size);
}
