#pragma once

#include "Timestamp.h"
#include "FileUtils.h"

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
