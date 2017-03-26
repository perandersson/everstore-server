#pragma once

#include "ESErrorCodes.h"
#include "IpcChild.h"

struct IpcChildProcess
{

	IpcChildProcess(const ChildProcessId id);

	virtual ~IpcChildProcess();

	// Wait for this process to close
	void waitAndClose();

	// Force-kill this process
	void kill();

	// Start this process
	ESErrorCode start(const string& command, const string& currentDirectory, const vector<string>& arguments);

	// Stop this process
	ESErrorCode stop();

	inline IpcChild& child() { return mChild; }

	inline const ChildProcessId id() const {
		return mChild.id();
	}

	inline process_t* handle() { return mChild.process(); }

private:

	// The client
	IpcChild mChild;
};

struct IpcChildProcesses : vector<IpcChildProcess*>
{
	IpcChildProcesses();

	~IpcChildProcesses();

	// Create a new process instance
	IpcChildProcess* createProcess();

	// Check to see if the supplied worker exists
	bool workerExists(const ChildProcessId id);

	// Wait and close all processes managed by this instance
	void waitAndClose();

	// Retrieves child process based on the worker ID
	IpcChildProcess* get(const ChildProcessId id);
};
