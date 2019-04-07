//
// Copyright (c) 2019 West Coast Code AB. All rights reserved.
//

#ifndef EVERSTORE_LOG_HPP
#define EVERSTORE_LOG_HPP

#include <string>
#include "../Process/ProcessID.h"

class Logger
{
public:
	virtual ~Logger() = default;

	/**
	 * Write a new log line to whatever logger is active
	 *
	 * @param level Log level
	 * @param str String we want to write to the log
	 */
	virtual void Write(uint32_t level, const std::string& str) volatile = 0;

protected:
	inline static bool IsErrorLevel(uint32_t level) {
		return level == 1;
	}

	inline static bool IsWarnLevel(uint32_t level) {
		return level == 2 || level == 3;
	}

	inline static bool IsInfoLevel(uint32_t level) {
		return level > 3 && level < 6;
	}

	inline static bool IsDebugLevel(uint32_t level) {
		return level > 5 && level < 10;
	}
};

class Log
{
public:
	static constexpr uint32_t Error = 1;
	static constexpr uint32_t Warn = 2;
	static constexpr uint32_t Warn2 = 3;
	static constexpr uint32_t Info = 4;
	static constexpr uint32_t Info2 = 5;

	// Level used for non-important log rows once every few frames
	static constexpr uint32_t Debug = 6;

	// Level used for logs happening one time per frame
	static constexpr uint32_t Debug2 = 7;

	// Level used for logs happening one or a few times per frame
	static constexpr uint32_t Debug3 = 8;

	// Level used for rows that happens multiple times per frame
	static constexpr uint32_t Debug4 = 9;

	/**
	 * Check to see if the logging level is at least the supplied level
	 *
	 * @param level
	 * @return
	 */
	static bool IsLogLevel(uint32_t level);

	/**
	 * Set the level of which logs are allowed to be written
	 *
	 * @param level The log level
	 */
	static void SetLogLevel(uint32_t level);

	/**
	 * Set the logger
	 *
	 * @param logger The logger
	 */
	static void SetLogger(Logger* logger);

	/**
	 *
	 * @param id
	 */
	static void SetChildProcessID(ProcessID id);

	/**
	 * Write something to the log
	 *
	 * @param level The log level
	 * @param str
	 * @param ...
	 */
	static void Write(uint32_t level, const char* str, ...);
};

#endif //EVERSTORE_LOG_HPP
