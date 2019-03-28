#include "Path.hpp"
#include <cstdio>

const string Path::StrPathDelim("/");

Path::Path()
		: value(), hash(0) {
}

Path::Path(const string& _value)
		: value(_value), hash(0) {
	if (_value.length() > 0) {
		hash = std::hash<string>{}(value);
	}
}

FILE* Path::Open() const {
	return Open("r+b");
}

FILE* Path::Open(const char* mode) const {
	string fileName = value;
#ifdef _WIN32
	StringUtils::replaceAll(fileName, '/', '\\');
#endif
	return fopen(fileName.c_str(), mode);
}

FILE* Path::OpenOrCreate(const char* mode) const {
	string fileName = value;
#ifdef _WIN32
	StringUtils::replaceAll(fileName, '/', '\\');
#endif
	// Create file if it does not exists
	auto tmpFile = fopen(fileName.c_str(), "a");
	fclose(tmpFile);
	return fopen(fileName.c_str(), mode);
}

ifstream Path::OpenStream() const {
	string fileName = value;
#ifdef _WIN32
	StringUtils::replaceAll(fileName, '/', '\\');
#endif
	ifstream file;
	file.open(fileName.c_str());
	return file;
}