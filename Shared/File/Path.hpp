#ifndef EVERSTORE_PATH_HPP_
#define EVERSTORE_PATH_HPP_

#include "../StringUtils.h"
#include <cstdio>
#include <fstream>

/**
 * Represents a path in the file-system. Path-delimiter is always '/' (forward-slash) from the <code>Path</code>'s
 * point of view. The path will be automatically converted at places where it's necessary.
 */
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
	 * If the path is for a file then open it in read-only mode
	 *
	 * @return
	 */
	FILE* Open() const;

	/**
	 * If the path is for a file then open it
	 *
	 * @param mode
	 * @return
	 */
	FILE* Open(const char* mode) const;

	/**
	 * Open the file if it exists; Creates the file if it doesn't.
	 *
	 * @param mode
	 * @return
	 */
	FILE* OpenOrCreate(const char* mode) const;

	/**
	 *
	 * @return
	 */
	ifstream OpenStream() const;

	/**
	 * Figure out the directory where this path is located
	 *
	 * @return
	 */
	Path GetDirectory() const;

	/**
	 * @return
	 */
	static Path GetWorkingDirectory();

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
		return Path(value + rhs);
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
		if (rhs.value[0] == '/')
			return Path(value + rhs.value);
		return Path(value + StrPathDelim + rhs.value);
	}

	inline bool operator==(const Path& rhs) const {
		if (hash != rhs.hash)
			return false;
		return value == rhs.value;
	}

	inline bool operator!=(const Path& rhs) const {
		if (hash == rhs.hash)
			return false;
		return value != rhs.value;
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
