//
// Copyright (c) 2019 West Coast Code AB. All rights reserved.
//

#ifndef EVERSTORE_UNIXSOCKET_HPP
#define EVERSTORE_UNIXSOCKET_HPP

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <sys/un.h>

class UnixSocket
{
	typedef int Ref;
	static constexpr auto Invalid = -1;
};


#endif //EVERSTORE_UNIXSOCKET_HPP
