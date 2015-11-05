#ifndef _EVERSTORE_IPC_CHILD_H_
#define _EVERSTORE_IPC_CHILD_H_

#include "ESHeader.h"
#include "Bytes.h"
#include "Process.h"
#include "ChildProcessId.h"

struct IpcChild {

	IpcChild(const ChildProcessId id);

	virtual ~IpcChild();

	ESErrorCode connectToHost();

	// Send a message over the IPC pipe
	ESErrorCode sendTo(const ESHeader* header);
	
	// Send a message over the IPC pipe
	ESErrorCode sendTo(const Bytes* bytes);

	// Retrieves this child's unique id
	inline const ChildProcessId id() const { return mId; }

	inline process_t* process() { return &mProcess; }

	// Logging
	void log(const char* str, ...);

	// Logging
	void error(const char* str, ...);

	// Log an error message
	void error(ESErrorCode err);

	// Close pipe
	void close();
	
private:
	const ChildProcessId mId;
	process_t mProcess;
	mutex mMutex;
};

#endif
