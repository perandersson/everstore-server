#include "IpcChild.h"
#include "../StringUtils.h"

IpcChild::IpcChild(ChildProcessId id) : mId(id) {
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

void IpcChild::log(const char* str, ...) {
	va_list arglist;
	va_start(arglist, str);
	char tmp[5096];
	vsprintf(tmp, str, arglist);
	va_end(arglist);

	printf("%02d [INFO]: %s\n", mId.value, tmp);
}

void IpcChild::error(const char* str, ...) {
	va_list arglist;
	va_start(arglist, str);
	char tmp[5096];
	vsprintf(tmp, str, arglist);
	va_end(arglist);

	printf("%02d [ERROR]: %s\n", mId.value, tmp);
}

void IpcChild::error(ESErrorCode err) {
	printf("%02d [ERROR]: %s\n", mId.value, parseErrorCode(err));
}

void IpcChild::close() {
	process_close(&mProcess);
}
