//
// Copyright (c) 2019 West Coast Code AB. All rights reserved.
//

#ifndef EVERSTORE_SOCKET_HPP
#define EVERSTORE_SOCKET_HPP

#include "Port.hpp"
#include "../ESErrorCodes.h"

#if defined(_WIN32)

#include "Win32/Win32Socket.hpp"

#else

#include "Unix/UnixSocket.hpp"

#endif

class Process;

class Socket
{
public:
	~Socket();

	static bool Initialize();

	static void Shutdown();

	/**
	 * @return
	 */
	inline ESErrorCode SetBlocking() { return OsSocket::SetBlocking(&mSocket); }

	/**
	 * @param millis
	 * @return
	 */
	inline ESErrorCode SetTimeout(uint32_t millis) { return OsSocket::SetTimeout(&mSocket, millis); }

	/**
	 * @return
	 */
	inline ESErrorCode SetNoDelay() { return OsSocket::SetNoDelay(&mSocket); }

	/**
	 * @param sizeInBytes
	 * @return
	 */
	inline ESErrorCode SetBufferSize(uint32_t sizeInBytes) { return OsSocket::SetBufferSize(&mSocket, sizeInBytes); }

	/**
	 * Destroy this socket's internal resources
	 *
	 * @remark This is automatically done when this instance is deleted
	 */
	inline ESErrorCode Destroy() { return OsSocket::Close(&mSocket); }

	/**
	 * @return <code>true</code> if this socket is destroyed
	 */
	inline bool IsDestroyed() const { return OsSocket::IsInvalid(&mSocket); }

	/**
	 * @param s The socket we want to verify
	 * @return <code>true</code> if this socket is destroyed
	 */
	inline static bool IsDestroyed(Socket* s) { return s == nullptr || OsSocket::IsInvalid(&s->mSocket); }

	/**
	 * Listen for incoming connections
	 *
	 * @param port THe port we want to listen to
	 * @return
	 */
	ESErrorCode Listen(Port port, uint32_t maxConnections);

	/**
	 *
	 * @return
	 */
	Socket* AcceptBlocking();

	/**
	 *
	 * @param buffer
	 * @param size
	 * @return
	 */
	int32_t ReceiveAll(char* buffer, uint32_t size);

	/**
	 *
	 * @tparam Max
	 * @param s
	 * @param length
	 * @return
	 */
	template<uint32_t Max>
	inline int32_t ReceiveString(string* s, uint32_t length) {
		length = length > Max ? Max : length;
		char temp[Max + 1] = {0};
		const auto readBytes = ReceiveAll(temp, length);
		if (readBytes == -1) {
			return -1;
		}
		*s = string(temp);
		return readBytes;
	}

	/**
	 *
	 * @param bytes
	 * @param size
	 * @return
	 */
	int32_t SendAll(const char* bytes, uint32_t size);

	/**
	 * @param process
	 * @return
	 */
	ESErrorCode ShareWithProcess(Process* process);

	/**
	 * Create a new blocking socket
	 *
	 * @param bufferSizeInBytes
	 * @return
	 */
	static Socket* CreateBlocking(uint32_t bufferSizeInBytes);

	/**
	 * @return <code>true</code> if the server is running on an os with little endian
	 */
	inline static bool IsLittleEndian() {
		int num = 1;
		return (*(char*) &num == 1);
	}

	inline uint32_t GetBufferSize() const { return mBufferSize; }

	inline OsSocket* GetHandle() { return &mSocket; }

private:
	Socket(OsSocket::Ref socket, uint32_t bufferSize);

private:
	OsSocket mSocket;
	uint32_t mBufferSize;
};


#endif //EVERSTORE_SOCKET_HPP
