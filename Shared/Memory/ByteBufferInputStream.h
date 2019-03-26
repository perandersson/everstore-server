#ifndef _EVERSTORE_BYTE_BUFFER_INPUT_STREAM_H_
#define _EVERSTORE_BYTE_BUFFER_INPUT_STREAM_H_

#include "ByteBuffer.h"

class ByteBufferInputStream
{
public:
	explicit ByteBufferInputStream(ByteBuffer* buffer);

	int32_t lastIndexOf(char c);

private:
	const char* mStart;
	const char* mEnd;
};

#endif
