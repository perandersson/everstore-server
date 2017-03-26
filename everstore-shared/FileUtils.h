#pragma once

#include <cstdio>
#include "StringUtils.h"

struct FileUtils
{
	// Represents an empty string
	static const char EMPTY;

	// Represents a space character
	static const char SPACE;

	// The size of a space character
	static const int SPACE_SIZE;

	// Represents a new line
	static const char NL;

	// Represents the size of a new line
	static const int NL_SIZE;

	// Represents a path delimiter
	static const string PATH_DELIM;

	/**
	* Returns the file size for the supplied file
	*
	* \return The file size; 0 if file does not exists
	*/
	static uint32_t getFileSize(FILE* file);

	/**
	 * Check to see if the supplied file exists
	 *
	 * @param filePath The path to the file
	 * @return <code>true</code> if the file exists
	 */
	static bool fileExists(const string& filePath);

	/**
	 * Truncate the file with the supplied filename - discards all other bytes
	 *
	 * @param filePath The path to the file
	 * @param newLength The maximum length of the file
	 */
	static void truncate(const string& filePath, long newLength);

	/**
	 * Truncates the supplied <code>FILE</code> pointer
	 *
	 * @param f
	 * @param newLength
	 */
	static void truncate(FILE* f, long newLength);

	/**
	 * @param filePath The path to the file
	 * @return The size of the file - 0 if no file is found.
	 */
	static uint32_t getFileSize(const string& filePath);

	/**
	 * Remove the file with the supplied path
	 *
	 * @param filePath The path to the file we want to remove
	 * @return
	 */
	static int remove(const string& filePath);

	/**
	 * Create a specific folder
	 *
	 * @param path
	 */
	static void createFolder(const string& path);

	/**
	 * Create all folders required for the supplied path to exist
	 *
	 * @param path The full path
	 */
	static void createFolders(const string& path);

	/**
	 * Change the current working directory for the application
	 *
	 * @param path The new application directory
	 * @return <code>true</code> if the change was successful.
	 */
	static bool setCurrentDirectory(const string& path);

	/**
	 * @return The path to the directory path where temporary files are located
	 */
	static string getTempDirectory();

	/**
	 * Generate a new path to a temporary file.
	 *
	 * @return
	 */
	static string getTempFile();

	/**
	 * Clean- and delete the content of the supplied directory
	 *
	 * @param path Directory path
	 */
	static void clearAndDeleteDirectory(const string& path);

	/**
	 * Copy the content of the supplied file into a new file
	 *
	 * @param srcFile The file we want to copy
	 * @param destFile Where to copy the file into
	 * @return <code>true</code> if the copy was successful
	 */
	static bool copyFile(const string& srcFile, const string& destFile);

	/**
	 * @return The generated filename of a temporary file
	 */
	static string getTempFileName();

	/**
	 * Gather all filenames that ends with a supplied suffix
	 *
	 * @param path
	 * @param suffix
	 * @return
	 */
	static vector<string> findFilesEndingWith(const string& path, const string& suffix);
};

