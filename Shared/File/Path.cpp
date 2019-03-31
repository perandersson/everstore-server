#include "Path.hpp"
#include "FileUtils.h"
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
	auto fp = fopen(fileName.c_str(), mode);
	if (fp) {
		return fp;
	}

	// Create the path to the file
	FileUtils::createFullForPath(value);

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

Path Path::GetDirectory() const {
	const auto idx = value.find_last_of('/');
	if (idx == string::npos) {
		return GetWorkingDirectory();
	} else if (idx == 0) {
		return Path("/");
	}
	return Path(value.substr(0, idx));
}

Path Path::GetWorkingDirectory() {
#if defined(_WIN32)
	char buffer[MAX_PATH];
	GetModuleFileName(nullptr, buffer, MAX_PATH);
	string::size_type pos = string(buffer).find_last_of("\\/");
	return Path(string(buffer).substr(0, pos));
#else
	char directory[1024];
	getcwd(directory, sizeof(directory));
	return Path(string(directory));
#endif
}
