#ifndef _EVERSTORE_STRING_UTILS_H_
#define _EVERSTORE_STRING_UTILS_H_

#include "es_config.h"

struct StringUtils {

	static string toString(int i) {
		char tmp[16] = { 0 };
		sprintf(tmp, "%d", i);
		return string(tmp);
	}

	static string toString(uint32_t i) {
		char tmp[16] = { 0 };
		sprintf(tmp, "%d", i);
		return string(tmp);
	}

	static string toString(uint16_t i) {
		return toString((uint32_t)i);
	}

	static uint16_t toUint16(const string& v) {
		return (uint16_t)atoi(v.c_str());
	}

	static uint32_t toUint32(const string& v) {
		return (uint32_t)atoi(v.c_str());
	}

	static void replaceAll(string& value, const string::value_type replace, const string::value_type newval) {
		const uint32_t size = value.size();
		for (uint32_t i = 0; i < size; ++i) {
			if (value[i] == replace) {
				value[i] = newval;
			}
		}
	}

	static bool endsWith(std::string const &fullString, std::string const &ending) {
		if (ending.empty())
			return true;
		if (fullString.length() >= ending.length()) {
			return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
		} else {
			return false;
		}
	}

	static void split(const string& str, const char delimiters, vector<string>& tokens)
	{
		string::size_type lastPos = str.find_first_not_of(delimiters, 0);
		string::size_type pos = str.find_first_of(delimiters, lastPos);

		while (string::npos != pos || string::npos != lastPos) {
			tokens.push_back(str.substr(lastPos, pos - lastPos));
			lastPos = str.find_first_not_of(delimiters, pos);
			pos = str.find_first_of(delimiters, lastPos);
		}
	}

};

#endif
