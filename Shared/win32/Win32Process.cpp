#include "Win32Process.h"
#include "Win32Socket.h"
#include "Win32Mutex.h"
#include "../Socket.h"
#include "../Message/ESHeader.h"

string PIPE_NAME_PREFIX = "\\\\.\\pipe\\everstore";
const uint32_t PIPE_BUFFER_SIZE_1MB = 1024 * 1024;
const uint32_t PIPE_DEFAULT_TIMEOUT_MS = 2000;

namespace
{
	string vector_to_string(const vector<string>& v) {
		string result;
		for (auto& arg : v) {
			result += (arg + string(" "));
		}
		return result;
	}
}

void process_init(process_t* p) {
	ZeroMemory(&p->processInfo, sizeof(PROCESS_INFORMATION));
	ZeroMemory(&p->startupInfo, sizeof(STARTUPINFO));
	p->startupInfo.cb = sizeof(STARTUPINFO);
#ifdef _DEBUG
	p->startupInfo.dwFlags |= STARTF_USESTDHANDLES;
	p->startupInfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
#endif
	p->running = false;
	p->pipe = INVALID_PIPE;
}

ESErrorCode _win32_pipe_open(const string& name, PIPE* p) {
	*p = INVALID_PIPE;

	SECURITY_ATTRIBUTES sec;
	sec.bInheritHandle = TRUE;
	sec.lpSecurityDescriptor = nullptr;
	sec.nLength = sizeof(SECURITY_ATTRIBUTES);

	*p = CreateNamedPipe(name.c_str(), PIPE_ACCESS_DUPLEX | PIPE_TYPE_BYTE | PIPE_READMODE_BYTE, PIPE_WAIT, 1,
	                     PIPE_BUFFER_SIZE_1MB * 2, PIPE_BUFFER_SIZE_1MB / 2,
	                     PIPE_DEFAULT_TIMEOUT_MS, &sec);

	if (*p == INVALID_HANDLE_VALUE)
		return ESERR_PIPE_CREATE;

	return ESERR_NO_ERROR;
}

ESErrorCode process_start(const string& name, const string& command, const string& currentDirectory,
                          const vector<string>& arguments, uint32_t, process_t* p) {
	const auto pipeName = PIPE_NAME_PREFIX + name;
	const ESErrorCode err = _win32_pipe_open(pipeName, &p->pipe);
	if (isError(err)) return err;

	const string c = currentDirectory + string("\\") + command + ".exe " + vector_to_string(arguments);
	SECURITY_ATTRIBUTES sec;
	sec.bInheritHandle = TRUE;
	sec.lpSecurityDescriptor = nullptr;
	sec.nLength = sizeof(SECURITY_ATTRIBUTES);
	if (!CreateProcess(nullptr, (char*) c.c_str(), &sec, &sec, TRUE, 0, nullptr, currentDirectory.c_str(), &p->startupInfo,
	                   &p->processInfo))
		return ESERR_PROCESS_CREATE_CHILD;

	p->running = true;
	return ConnectNamedPipe(p->pipe, nullptr) == TRUE ? ESERR_NO_ERROR : ESERR_PIPE_LISTEN;
}

ESErrorCode process_close(process_t* p) {
	if (p->pipe != INVALID_PIPE) {
		CloseHandle(p->pipe);
		p->pipe = INVALID_PIPE;
	}

	if (p->running) {
		DWORD exitCode = 0;
		if (GetExitCodeProcess(p->processInfo.hProcess, &exitCode) == TRUE && exitCode == STILL_ACTIVE) {
			CloseHandle(p->processInfo.hProcess);
			CloseHandle(p->processInfo.hThread);
			// TODO: Double-check that this does not turn into a memory-leak
		}
		p->running = false;
	}

	return ESERR_NO_ERROR;
}

ESErrorCode process_wait(process_t* p) {
	// If the application is still running then wait for it to complete
	DWORD exitCode = 0;
	if (GetExitCodeProcess(p->processInfo.hProcess, &exitCode) == TRUE && exitCode == STILL_ACTIVE) {
		WaitForSingleObject(p->processInfo.hProcess, INFINITE);
	}
	return ESERR_NO_ERROR;
}

ESErrorCode process_connect_to_host(const string& name, process_t* p) {
	p->pipe = INVALID_PIPE;

	SECURITY_ATTRIBUTES sec;
	sec.bInheritHandle = TRUE;
	sec.lpSecurityDescriptor = nullptr;
	sec.nLength = sizeof(SECURITY_ATTRIBUTES);

	const auto pipeName = PIPE_NAME_PREFIX + name;
	p->pipe = CreateFile(pipeName.c_str(), GENERIC_READ | GENERIC_WRITE, TRUE, &sec, OPEN_EXISTING, 0, nullptr);
	if (p->pipe == INVALID_PIPE) {
		return ESERR_PIPE_CONNECT;
	}

	return ESERR_NO_ERROR;
}

uint32_t process_read(process_t* p, _OUT char* bytes, uint32_t size) {
	DWORD totalBytes = 0;
	BOOL result;
	do {
		DWORD readBytes = 0;
		result = ReadFile(p->pipe, &bytes[totalBytes], size - totalBytes, &readBytes, nullptr);
		totalBytes += readBytes;
	} while (result && totalBytes < size);
	return (uint32_t) totalBytes;
}

uint32_t process_write(process_t* p, const char* bytes, uint32_t size) {
	DWORD totalBytes = 0;
	BOOL result;
	do {
		DWORD writeBytes = 0;
		result = WriteFile(p->pipe, &bytes[totalBytes], size - totalBytes, &writeBytes, nullptr);
		totalBytes += writeBytes;
	} while (result && totalBytes < size);
	if (totalBytes > 0)
		FlushFileBuffers(p->pipe);
	return (uint32_t) totalBytes;
}

ESErrorCode process_share_socket(process_t* p, SOCKET hostSocket, mutex_t lock) {
	ESHeader header;
	header.type = REQ_NEW_CONNECTION;
	header.client = hostSocket;
	if (process_write(p, (char*) &header, sizeof(header)) == 0)
		return ESERR_PIPE_WRITE;

	WSAPROTOCOL_INFO info;
	if (WSADuplicateSocket(hostSocket, p->processInfo.dwProcessId, &info) == SOCKET_ERROR) {
		closesocket(hostSocket);
		return ESSER_PROCESS_SHARE_SOCKET;
	}

	if (process_write(p, (char*) &info, sizeof(info)) == 0) {
		closesocket(hostSocket);
		return ESERR_PIPE_WRITE;
	}

	mutex_t duplicatedHandle;
	BOOL result = DuplicateHandle(GetCurrentProcess(), lock, p->processInfo.hProcess, &duplicatedHandle, 0, TRUE,
	                              DUPLICATE_SAME_ACCESS);
	if (!result) {
		closesocket(hostSocket);
		return ESERR_PIPE_WRITE;
	}

	if (process_write(p, (char*) &duplicatedHandle, sizeof(mutex_t)) == 0) {
		closesocket(hostSocket);
		return ESERR_PIPE_WRITE;
	}

	return ESERR_NO_ERROR;
}

SOCKET process_accept_shared_socket(process_t* p, mutex_t* lock) {
	*lock = INVALID_LOCK;

	// Read the spared socket protocol information
	WSAPROTOCOL_INFO info;
	if (process_read(p, (char*) &info, sizeof(info)) == 0)
		return INVALID_SOCKET;

	// Read the lock associated with the socket
	mutex_t l;
	if (process_read(p, (char*) &l, sizeof(mutex_t)) == 0 || l == INVALID_LOCK)
		return ESERR_PROCESS_ATTACH_SHARED_SOCKET;

	// Create a socket
	SOCKET s = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, &info, 0, 0);

	// Make sure to block
	ESErrorCode err = socket_setblocking(s);
	if (isError(err)) {
		socket_close(s);
		return INVALID_SOCKET;
	}

	*lock = l;
	return s;
}
