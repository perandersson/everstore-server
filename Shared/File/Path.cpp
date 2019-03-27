#include "Path.hpp"
#include <cstdio>

const string Path::StrPathDelim = string(1, FileUtils::PATH_DELIM);

Path::Path()
		: value(), hash(0) {
}

Path::Path(const string& _value)
		: value(_value), hash(0) {
	if (_value.length() > 0) {
#ifdef _WIN32
		StringUtils::replaceAll(value, '/', '\\');
#endif
		hash = std::hash<string>{}(value);
	}
}
