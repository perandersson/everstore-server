#ifndef _EVERSTORE_HEADER_H_
#define _EVERSTORE_HEADER_H_

#include "../es_config.h"
#include "ESRequestType.h"
#include "../Socket/Socket.hpp"
#include "../Socket/Socket.hpp"
#include "../Process/ProcessID.h"

typedef uint32_t ESHeaderProperties;

enum ESHeaderProperty {
	ESPROP_NONE = 0,

	ESPROP_MULTIPART = 1u,
	
	ESPROP_COMPRESSED = 2u,

	ESPROP_INCLUDE_TIMESTAMP = 4u
};

// Header for all messages sent to the server
struct ESHeader {
	ESRequestType type;
	int32_t size;
	uint32_t requestUID;
	ESHeaderProperties properties;

	union {
		OsSocket::Ref client;
		ProcessID workerId;
	};

	ESHeader() : type(REQ_INVALID), size(0), requestUID(0), properties(ESPROP_NONE), workerId(0) {}
	ESHeader(ESRequestType type, int32_t size, uint32_t requestUID, ESHeaderProperties properties, ProcessID workerId) :
		type(type), size(size), requestUID(requestUID), properties(properties), workerId(workerId) {}
    ESHeader(ESRequestType type, int32_t size, uint32_t requestUID, ESHeaderProperties properties, OsSocket::Ref socket) :
            type(type), size(size), requestUID(requestUID), properties(properties), client(socket) {}
	~ESHeader(){}
};

static_assert(sizeof(ESHeader) == 20, "Invalid size for the ESHeader object");

// API version
static const uint32_t VERSION = 1;

// Represents an invalid header
extern ESHeader INVALID_HEADER;

#endif