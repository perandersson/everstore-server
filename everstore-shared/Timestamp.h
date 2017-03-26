#pragma once

#include <cinttypes>

/**
 * A timestamp object that represents the current time in UTC+0 in a format that looks like this:
 *
 * 2015-07-06T18:26:59.483<nullptr>
 */
struct Timestamp
{
	static const uint32_t MAX_LENGTH = 24;
	static const uint32_t BYTES_LENGTH = MAX_LENGTH - 1;
	static const uint32_t FRACTAL_POS = 20;

	char value[MAX_LENGTH];

	Timestamp();

	~Timestamp();
};
