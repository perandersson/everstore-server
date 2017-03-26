#pragma once

#include <string>
#include <vector>
#include <cinttypes>

using std::string;
using std::vector;

struct StringUtils
{
	/**
	 * Convert an unsigned 32 bit value to a string
	 *
	 * @param i The integer value
	 * @return The string
	 */
	static string toString(int32_t i);

	/**
	 * Convert a signed 32 bit value to a string
	 *
	 * @param i The integer value
	 * @return The string
	 */
	static string toString(uint32_t i);

	/**
	 * Convert a 16 bit unsigned short value into a string
	 *
	 * @param i The short (16 bit) value
	 * @return A string
	 */
	static string toString(uint16_t i);

	/**
	 * Convert a string into an unsigned 16 bit integer
	 *
	 * @param v The string
	 * @return The unsigned 16 bit value
	 */
	static uint16_t toUint16(const string& v);

	/**
	 * Convert a string into an unsigned 32 bit integer
	 *
	 * @param v The string
	 * @return The unsigned 16 bit value
	 */
	static uint32_t toUint32(const string& v);

	/**
	 * Replace all occurrences of a supplied characterwith another character
	 *
	 * @param value The original value
	 * @param replace The character we want to replace
	 * @param newval The replacement character
	 */
	static void replaceAll(string& value, const string::value_type replace, const string::value_type newval);

	/**
	 * Check to see if the supplied string ends with another string
	 *
	 * @param fullString The string we want to compare with
	 * @param ending
	 * @return
	 */
	static bool endsWith(const string& fullString, const string& ending);

	/**
	 * Split the supplied string into multiple tokens with a delimiter
	 *
	 * @param str The string value we want to split into tokens
	 * @param delimiters The delimiter
	 * @param tokens where to put the generated tokens
	 */
	static void split(const string& str, const char delimiters, vector<string>& tokens);
};
