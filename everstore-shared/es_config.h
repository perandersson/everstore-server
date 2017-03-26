#pragma once

#ifndef NDEBUG
#   define _DEBUG
#endif

#if defined(_WIN32) || defined(_WIN64)
#   define ES_WINDOWS
#elif defined(__GNUC__)
#   define ES_GCC
#else
#   error Unknown Compiler
#endif

#if defined(ES_WINDOWS)
#	if _WIN64
#		define ENVIRONMENT64
#	else
#		define ENVIRONMENT32
#	endif
#elif defined(ES_GCC)
#	if __x86_64__ || __ppc64__
#		define ENVIRONMENT64
#	else
#		define ENVIRONMENT32
#	endif
#endif

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
