//
// Created by Per on 2015-07-08.
//

#ifndef EVENTSTORE_FILEUTILS_H
#define EVENTSTORE_FILEUTILS_H

#include "es_config.h"
#include "StringUtils.h"

using namespace std;

struct FileUtils {
	static const char EMPTY;
	static const char SPACE;
	static const int SPACE_SIZE;
	static const char NL;
	static const int NL_SIZE;
	static const char* PATH_DELIM;


	/**
	* Returns the file size for the supplied file
	*
	* \return The file size; 0 if file does not exists
	*/
	static uint32_t getFileSize(FILE* file) {
		const auto begin = ftell(file);
		fseek(file, 0, SEEK_END);
		const auto end = ftell(file);
		fseek(file, 0, SEEK_SET);
		return (uint32_t)(end - begin);
	}

	static char* empty() {
		return (char*)&EMPTY;
	}

	static bool fileExists(const string& fileName) {
		FILE* f = fopen(fileName.c_str(), "r");
		if (f != NULL) fclose(f);
		return f != 0;
	}

	static void truncate(const string& fileName, long newLength);

	static void truncate(FILE* f, long newLength);

	// 
	// Returns the file size for file with the supplied filename
	static uint32_t getFileSize(const string& fileName) {
		FILE* file = fopen(fileName.c_str(), "r+b");
		if (file != 0) {
			auto size = getFileSize(file);
			fclose(file);
			return size;
		}
		else return 0;
	}

	static int remove(const string& fileName) {
		return ::remove(fileName.c_str());
	}

	static void createFolder(const string& path);

	static void createFullForPath(const string& path) {
		vector<string> paths;
		StringUtils::split(path, '/', paths);
		if (paths.size() > 1) {
			string totalPath;
			const uint32_t size = paths.size() - 1;
			for (uint32_t i = 0; i < size; ++i) {
				totalPath += paths[i] + string(FileUtils::PATH_DELIM);
				FileUtils::createFolder(totalPath);
			}
		}
	}

	// Set the current directory for the application
	static bool setCurrentDirectory(const string& path);

	// Retrieves the path to the directory path
	static string getTempDirectory();

	// Retrieves a path to a temporary file (will be different every time)
	static string getTempFile();

	// Clear the target directory
	static void clearAndDeleteDirectory(const string& path);

	// 
	static bool copyFile(const string& srcFile, const string& destFile);

	// 
	static string getTempFileWithoutPath();

	static vector<string> findFilesEndingWith(const string& path, const string& sufix);
};



#endif //EVENTSTORE_FILEUTILS_H
