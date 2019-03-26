#ifndef _EVERSTORE_TIMESTAMP_H_
#define _EVERSTORE_TIMESTAMP_H_

#include "../es_config.h"


/**
 * Object representing a timestamp
 * <p />
 * The format of the timestamp value looks like this: "2015-07-06T18:26:59.483&lt;nullptr&gt;"
 */
struct Timestamp
{
	static const uint32_t MaxLength = 24;
	static const uint32_t BytesLength = MaxLength - 1;
	static const uint32_t FractalPos = 20;

	char value[MaxLength];

	Timestamp();
};

#endif
