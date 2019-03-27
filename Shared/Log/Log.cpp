//
// Copyright (c) 2019 West Coast Code AB. All rights reserved.
//

#include "Log.hpp"
#include "../Database/Timestamp.h"
#include <iostream>
#include <cstdarg>

namespace
{
	uint32_t gLogLevel = Log::Info;
	string gChildProcessID("00");

	class DefaultLogger : public Logger
	{
	public:
		inline void Write(uint32_t level, const std::string& str) volatile final {
			Timestamp now;
			if (IsErrorLevel(level)) {
				std::cerr << now.value << " [" << gChildProcessID << "E] " << str << std::endl;
			} else if (IsWarnLevel(level)) {
				std::cout << now.value << " [" << gChildProcessID << "W] " << str << std::endl;
			} else if (IsInfoLevel(level)) {
				std::cout << now.value << " [" << gChildProcessID << "I] " << str << std::endl;
			} else if (IsDebugLevel(level)) {
				std::cout << now.value << " [" << gChildProcessID << "D] " << str << std::endl;
			}
		}
	} gDefaultStdLogger;

	volatile Logger* gCurrentLogger = &gDefaultStdLogger;
}

void Log::SetLogLevel(uint32_t level) {
	gLogLevel = level;
}

void Log::SetLogger(Logger* logger) {
	if (logger == nullptr)
		gCurrentLogger = &gDefaultStdLogger;
	else
		gCurrentLogger = logger;
}

bool Log::IsLogLevel(uint32_t level) {
	return level <= gLogLevel;
}

void Log::Write(uint32_t level, const char* str, ...) {
	if (level > gLogLevel) {
		return;
	}

	va_list arglist;
	va_start(arglist, str);
	char tmp[5096];
	vsprintf(tmp, str, arglist);
	va_end(arglist);

	gCurrentLogger->Write(level, std::string(tmp));
}

void Log::SetChildProcessID(ChildProcessID id) {
	char tmp[128];
	sprintf_s(tmp, 128, "%02d", id);
	gChildProcessID = string(tmp);
}
