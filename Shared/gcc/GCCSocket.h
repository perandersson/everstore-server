#ifndef _EVERSTORE_SOCKET_H_
#define _EVERSTORE_SOCKET_H_

#include "../es_config.h"
#include "../ESErrorCodes.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#define INVALID_SOCKET -1

#if defined(ENVIRONMENT32)
#define SOCKET int
#else
#define SOCKET int
#endif

#define socket_close ::close

#endif
