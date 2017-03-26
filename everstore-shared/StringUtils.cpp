#include "StringUtils.h"

string StringUtils::toString(int32_t i) {
	char tmp[16] = {0};
	sprintf(tmp, "%d", i);
	return string(tmp);
}

string StringUtils::toString(uint32_t i) {
	char tmp[16] = {0};
	sprintf(tmp, "%d", i);
	return string(tmp);
}

string StringUtils::toString(uint16_t i) {
	return toString((uint32_t) i);
}

uint16_t StringUtils::toUint16(const string& v) {
	return (uint16_t) atoi(v.c_str());
}

uint32_t StringUtils::toUint32(const string& v) {
	return (uint32_t) atoi(v.c_str());
}

void StringUtils::replaceAll(string& value, const string::value_type replace, const string::value_type newval) {
	const string::size_type size = value.size();
	for (string::size_type i = 0; i < size; ++i) {
		if (value[i] == replace) {
			value[i] = newval;
		}
	}
}

bool StringUtils::endsWith(const std::string& fullString, const std::string& ending) {
	if (ending.empty())
		return true;
	if (fullString.length() >= ending.length()) {
		return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
	} else {
		return false;
	}
}

void StringUtils::split(const string& str, const char delimiters, vector<string>& tokens) {
	string::size_type lastPos = str.find_first_not_of(delimiters, 0);
	string::size_type pos = str.find_first_of(delimiters, lastPos);

	while (string::npos != pos || string::npos != lastPos) {
		tokens.push_back(str.substr(lastPos, pos - lastPos));
		lastPos = str.find_first_not_of(delimiters, pos);
		pos = str.find_first_of(delimiters, lastPos);
	}
}
