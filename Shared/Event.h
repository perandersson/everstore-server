#ifndef _EVERSTORE_EVENT_H_
#define _EVERSTORE_EVENT_H_

#include "es_config.h"
#include "Database/Timestamp.h"
#include "File/FileUtils.h"

struct Event {
	Timestamp timestamp;
	string data;

	inline long size() const {
		long size = 0;
		size += Timestamp::BYTES_LENGTH;
		size += FileUtils::SPACE_SIZE;
		size += data.length();
		return size;
	}
};

#endif
