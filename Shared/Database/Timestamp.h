#ifndef _EVERSTORE_TIMESTAMP_H_
#define _EVERSTORE_TIMESTAMP_H_

#include "../es_config.h"

//
// Object representing a timestamp
//
// The format of the timestamp value looks like this:
// 2015-07-06T18:26:59.483<nullptr>
//
struct Timestamp {
	static const uint32_t MAX_LENGTH = 24;
	static const uint32_t BYTES_LENGTH = MAX_LENGTH - 1;
	static const uint32_t FRACTAL_POS = 20;

	char value[MAX_LENGTH];

	Timestamp();
	~Timestamp();
};

#endif
