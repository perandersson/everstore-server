#ifndef _EVERSTORE_IPC_CHILD_PROCESS_H_
#define _EVERSTORE_IPC_CHILD_PROCESS_H_

#include "../ESErrorCodes.h"
#include "IpcChild.h"

class IpcChildProcess
{
public:
	IpcChildProcess(ChildProcessID id, uint32_t maxBufferSize);

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

	inline ChildProcessID id() const { return mChild.id(); }

	inline process_t* handle() { return mChild.process(); }

private:
	IpcChild mChild;
	const uint32_t mMaxBufferSize;
};

#endif
