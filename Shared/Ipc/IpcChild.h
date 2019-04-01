#ifndef _EVERSTORE_IPC_CHILD_H_
#define _EVERSTORE_IPC_CHILD_H_

#include "../Message/ESHeader.h"
#include "../Memory/ByteBuffer.h"
#include "../Process/Process.hpp"
#include "../Mutex/Mutex.hpp"

class IpcChild
{
public:
	explicit IpcChild(ProcessID id, Process* process);

	~IpcChild();

	// Send a message over the IPC pipe
	ESErrorCode sendTo(const ESHeader* header);

	// Send a message over the IPC pipe
	ESErrorCode sendTo(const ByteBuffer* bytes);

	int32_t read(char* bytes, uint32_t size);

	// Retrieves this child's unique id
	inline ProcessID id() const { return mId; }

	inline Process* process() { return mProcess; }

	// Close pipe
	void close();

private:
	const ProcessID mId;
	Process* mProcess;
	mutex mMutex;
};

#endif
