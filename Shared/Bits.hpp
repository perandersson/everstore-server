//
// Copyright (c) 2019 West Coast Code AB. All rights reserved.
//

#ifndef EVERSTORE_BITS_HPP
#define EVERSTORE_BITS_HPP


#include <cinttypes>
#include <string>

using std::string;

struct Bits
{
	typedef uint64_t Type;

	static constexpr Type None = 0u;

	static constexpr Type All = UINT64_MAX;

	struct BuiltIn
	{
		/**
		 * The key used that represents for when a journal is created
		 */
		static const string NewJournalKey;

		/**
		 *
		 */
		static constexpr Type NewJournalBit = 1u;
	};

	inline static Type Set(Type value, Type bits) noexcept {
		return value | bits;
	}

	inline static Type Unset(Type value, Type bits) noexcept {
		return (value & (~bits));
	}

	inline static bool IsSet(Type value, Type bits) noexcept {
		return (value & bits) == bits;
	}
};


#endif //EVERSTORE_BITS_HPP
