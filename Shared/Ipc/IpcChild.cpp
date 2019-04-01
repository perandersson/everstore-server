#include "IpcChild.h"
#include "../StringUtils.h"

IpcChild::IpcChild(ProcessID id, Process* process)
		: mId(id), mProcess(process) {
}

IpcChild::~IpcChild() {
	if (mProcess) {
		delete mProcess;
		mProcess = nullptr;
	}
}

ESErrorCode IpcChild::sendTo(const ESHeader* header) {
	const uint32_t size = sizeof(ESHeader);
	lock_guard<mutex> l(mMutex);
	const auto bytesSent = mProcess->Write((char*) header, size);
	if (bytesSent != size) return ESERR_PIPE_WRITE;
	return ESERR_NO_ERROR;
}

ESErrorCode IpcChild::sendTo(const ByteBuffer* bytes) {
	assert(bytes != nullptr);

	// The offset of the memory represents the size of the memory
	const auto size = bytes->offset();

	// Send the data to the supplied process
	mMutex.lock();
	const auto ret = mProcess->Write(bytes->ptr(), size);
	mMutex.unlock();

	if (ret != size) {
		return ESERR_PIPE_WRITE;
	}

	return ESERR_NO_ERROR;
}

void IpcChild::close() {
	mProcess->Destroy();
}

int32_t IpcChild::read(char* bytes, uint32_t size) {
	return mProcess->Read(bytes, size);
}
