#ifndef _EVERSTORE_ESCONFIG_H_
#define _EVERSTORE_ESCONFIG_H_

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <windows.h>
#else
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#endif
#include <signal.h>
#include <stddef.h>

// Check windows
#if _WIN32 || _WIN64
#	if _WIN64
#		define ENVIRONMENT64
#	else
#		define ENVIRONMENT32
#	endif
#endif

// Check GCC
#if __GNUC__
#	if __x86_64__ || __ppc64__
#		define ENVIRONMENT64
#	else
#		define ENVIRONMENT32
#	endif
#endif

#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <string>
#include <cstdio>
#include <mutex>
#include <thread>
#include <list>
#include <vector>
#include <queue>
#include <unordered_map>
#include <atomic>
#include <sstream>
#include <memory>
#include <cassert>

// Used to declare that the parameter is an output parameter
#define _OUT

using namespace std;

typedef uint64_t bit_mask;

#ifndef BIT
#define BIT(x) 1 << x
#endif

#ifndef BIT_ISSET
#define BIT_ISSET(a, b) (( a & b ) != 0)
#endif

#ifndef BIT_UNSET
#define BIT_UNSET(a, b) (a &= ~b)
#endif

#ifndef BIT_SET
#define BIT_SET(a, b) (a |= (b))
#endif

#ifndef BIT_ALL
#define BIT_ALL UINT_MAX
#endif

#ifndef BIT_NONE
#define BIT_NONE 0
#endif

#ifndef BOOL
#define BOOL int
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#endif
