#ifndef _EVERSTORE_BYTE_BUFFER_INPUT_STREAM_H_
#define _EVERSTORE_BYTE_BUFFER_INPUT_STREAM_H_

#include "Bytes.h"

struct ByteBufferInputStream {

	ByteBufferInputStream(Bytes* buffer);

	~ByteBufferInputStream();

	int32_t lastIndexOf(char c);

private:
	const char* mStart;
	const char* mEnd;
};

#endif
