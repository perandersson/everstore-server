#ifndef EVERSTORE_PATH_HPP_
#define EVERSTORE_PATH_HPP_

#include "../StringUtils.h"
#include "FileUtils.h"
#include <cstdio>

struct Path
{
	/**
	 * Delimited between each name in a file path in string format
	 */
	static const string StrPathDelim;

	string value;
	size_t hash;

	Path();

	explicit Path(const string& value);

	Path(const Path& rhs) = default;

	/**
	 *
	 * @param rhs
	 * @return
	 */
	inline Path operator+(const string& rhs) const {
		if (value.empty())
			return Path(rhs);
		if (rhs.empty())
			return *this;
		if (rhs[0] == '/')
			return Path(value + rhs);
		return Path(value + StrPathDelim + rhs);
	}

	/**
	 * @param rhs
	 * @return
	 */
	inline Path operator+(const Path& rhs) const {
		if (value.empty())
			return Path(rhs);
		if (rhs.value.empty())
			return *this;
		if (rhs.value[0] == FileUtils::PATH_DELIM)
			return Path(value + rhs.value);
		return Path(value + StrPathDelim + rhs.value);

	}
};

namespace std
{
	template<>
	struct hash<Path>
	{
		std::size_t operator()(const Path& k) const {
			return k.hash;
		}
	};
}

#endif
