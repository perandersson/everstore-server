#ifndef _EVERSTORE_IPC_CHILD_H_
#define _EVERSTORE_IPC_CHILD_H_

#include "../Message/ESHeader.h"
#include "../Memory/ByteBuffer.h"
#include "../Process.h"
#include "ChildProcessID.h"

struct IpcChild {

	IpcChild(ChildProcessID id);

	virtual ~IpcChild();

	ESErrorCode connectToHost();

	// Send a message over the IPC pipe
	ESErrorCode sendTo(const ESHeader* header);
	
	// Send a message over the IPC pipe
	ESErrorCode sendTo(const ByteBuffer* bytes);

	// Retrieves this child's unique id
	inline ChildProcessID id() const { return mId; }

	inline process_t* process() { return &mProcess; }

	// Close pipe
	void close();
	
private:
	const ChildProcessID mId;
	process_t mProcess;
	mutex mMutex;
};

#endif
