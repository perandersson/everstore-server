#pragma once

#include "es_config.h"
#include "ESRequestType.h"
#include "Socket.h"
#include "ChildProcessId.h"
#include <cinttypes>

// Fail-safe to ensure that the sizes are as we expect them
static_assert(sizeof(uint16_t) == 2, "A 16 bit integer should be 2 bytes");
static_assert(sizeof(uint32_t) == 4, "A 32 bit integer should be 4 bytes");
static_assert(sizeof(uint64_t) == 8, "A 64 bit integer should be 8 bytes");

#ifdef __GNUC__
#define PACKED(class_to_pack) class_to_pack __attribute__((packed, aligned(1)))
#else
#define PACKED( class_to_pack ) __pragma( pack(push, 1) ) class_to_pack __pragma( pack(pop) )
#endif

typedef uint32_t ESHeaderProperties;

enum ESHeaderProperty
{
	ESPROP_NONE = 0,

	ESPROP_MULTIPART = BIT(0),

	ESPROP_COMPRESSED = BIT(1),

	ESPROP_INCLUDE_TIMESTAMP = BIT(2)
};

#if defined(ENVIRONMENT32)
typedef uint32_t workerid_t;
#elif defined(ENVIRONMENT64)
typedef uint64_t workerid_t;
#endif

// Header for all messages sent to the server
PACKED(struct ESHeader
		       {
			       ESRequestType type;
			       uint32_t size;
			       uint32_t requestUID;
			       ESHeaderProperties properties;
			       union
			       {
				       SOCKET client;
				       workerid_t workerId;
			       };
		       });

#if defined(ENVIRONMENT32)
static_assert(sizeof(ESHeader) == 20, "Invalid size for the ESHeader object");
#elif defined(ENVIRONMENT64)
static_assert(sizeof(ESHeader) == 24, "Invalid size for the ESHeader object");
#endif

// Header API version
static const uint32_t VERSION = 1;

// Represents an invalid header
extern ESHeader INVALID_HEADER;
