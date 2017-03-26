#pragma once

#include <cinttypes>
#include <string>

#include "es_config.h"
#include "ESErrorCodes.h"

using std::string;

#if defined(ES_WINDOWS)

#   include <winsock2.h>
#   include <windows.h>

#elif defined(ES_GCC)

#   include "gcc/GCCSocket.h"

#   include <sys/socket.h>
#   include <sys/ioctl.h>
#   include <sys/fcntl.h>
#   include <sys/un.h>
#   include <netinet/in.h>
#   include <netinet/tcp.h>
#   include <arpa/inet.h>
#   include <netdb.h>

#ifndef SOCKET
#   define SOCKET int
#endif

#endif

/**
 * @return <true>If this machine handles numbers in little-endian mode
 */
bool is_little_endian();

struct Properties;

/**
 * A class that represents a sharable socket
 */
class SharableSocket
{
public:
	SharableSocket(SOCKET socket, uint32_t bufferSize);

	~SharableSocket();

	/*
	 * Accept a new sharable and blocking socket
	 */
	SharableSocket* accept();

	template<uint32_t max>
	inline uint32_t receiveClamped(string* s, uint32_t length) {
		// Clamp length
		length = length >= max ? max - 1 : length;

		// Read the characters
		char temp[max] = {0};
		uint32_t recv = receive(temp, length);
		if (recv == 0) return 0;

		// Set and return the string
		*s = string(temp);
		return recv;
	}

	template<typename T>
	inline uint32_t receiveObject(T* object) {
		return receive((char*) object, (uint32_t) sizeof(T));
	}

	template<typename T>
	inline uint32_t sendObject(const T* object) {
		return send((const char*) object, (uint32_t) sizeof(T));
	}

	uint32_t receive(char* bytes, uint32_t size);

	uint32_t send(const char* bytes, uint32_t size);

public:
	/**
	 * Initialize the socket library
	 */
	static void init();

	/**
	 * Cleanup the socket library
	 */
	static void cleanup();

	/**
	 * Create a new socket
	 *
	 * @param p
	 * @return
	 */
	static SharableSocket* create(const Properties& p);

private:
	static bool blocking(SOCKET socket);

	static void close(SOCKET socket);

	static bool nodelay(SOCKET socket);

	static bool bufferSize(SOCKET socket, uint32_t sizeInBytes);

	static bool setTimeout(SOCKET socket, uint32_t millis);

private:
	SOCKET mSocket;
	uint32_t mBufferSize;
};

