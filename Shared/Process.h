#ifndef _EVERSTORE_ES_PROCESS_H_
#define _EVERSTORE_ES_PROCESS_H_

#ifdef WIN32
#include "win32/Win32Process.h"
#else
#include "gcc/GCCProcess.h"
#endif
#include "Message/ESHeader.h"
#include "Mutex.h"

// Prefix for the named pipe
extern string PIPE_NAME_PREFIX;

// Initialize the process
void process_init(process_t* p);

// Start a new process with a two-way pipe
ESErrorCode process_start(const string& name, const string& command, const string& currentDirectory, const vector<string>& arguments, process_t* p);

// Close and wait for the supplied process to stop
ESErrorCode process_close(process_t* p);

// Wait for the supplied process to stop
ESErrorCode process_wait(process_t* p);

ESErrorCode process_connect_to_host(const string& name, process_t* p);

uint32_t process_write(process_t* p, const char* bytes, uint32_t size);

uint32_t process_read(process_t* p, _OUT char* bytes, uint32_t size);

// Share a socket between the host and the child process
ESErrorCode process_share_socket(process_t* p, SOCKET hostSocket, mutex_t hostSocketLock);

// Accept a shared socket from the host process
SOCKET process_accept_shared_socket(process_t* p, const ESHeader* header, mutex_t* socketLock);

#endif
