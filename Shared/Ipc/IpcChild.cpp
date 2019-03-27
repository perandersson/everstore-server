#include "IpcChild.h"
#include "../StringUtils.h"

IpcChild::IpcChild(ChildProcessID id) : mId(id) {
}

IpcChild::~IpcChild() {

}

ESErrorCode IpcChild::connectToHost() {
	const auto name = PIPE_NAME_PREFIX + mId.toString();
	return process_connect_to_host(name, &mProcess);
}

ESErrorCode IpcChild::sendTo(const ESHeader* header) {
	const uint32_t size = sizeof(ESHeader);
	lock_guard<mutex> l(mMutex);
	const auto bytesSent = process_write(&mProcess, (char*)header, size);
	if (bytesSent != size) return ESERR_PIPE_WRITE;
	return ESERR_NO_ERROR;
}

ESErrorCode IpcChild::sendTo(const ByteBuffer* bytes) {
	assert(bytes != nullptr);

	// The offset of the memory represents the size of the memory
	const auto size = bytes->offset();
	
	// Send the data to the supplied process
	mMutex.lock();
	const auto ret = process_write(&mProcess, bytes->ptr(), size);
	mMutex.unlock();
	
	// Verify result
	if (ret != size) return ESERR_PIPE_WRITE;
	return ESERR_NO_ERROR;
}

void IpcChild::close() {
	process_close(&mProcess);
}
