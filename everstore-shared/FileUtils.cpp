#include "FileUtils.h"
#include "es_config.h"

const char FileUtils::EMPTY = '\0';
const char FileUtils::SPACE = ' ';
const int FileUtils::SPACE_SIZE = 1;
const char FileUtils::NL = '\n';
const int FileUtils::NL_SIZE = 1;
#ifdef ES_WINDOWS
const string FileUtils::PATH_DELIM("\\");
#elif ES_GCC
const string FileUtils::PATH_DELIM("/");
#endif

#ifdef ES_WINDOWS

#   define WIN32_LEAN_AND_MEAN

#   include <Windows.h>
#   include <io.h>

#elif ES_GCC

#   include <sys/stat.h>

#endif

uint32_t FileUtils::getFileSize(FILE* file) {
	const auto begin = ftell(file);
	fseek(file, 0, SEEK_END);
	const auto end = ftell(file);
	fseek(file, 0, SEEK_SET);
	return (uint32_t) (end - begin);
}

bool FileUtils::fileExists(const string& filePath) {
	FILE* f = fopen(filePath.c_str(), "r");
	if (f != NULL) fclose(f);
	return f != 0;
}

void FileUtils::truncate(const string& filePath, long newLength) {
	FILE* f = fopen(filePath.c_str(), "r+b");
	if (f != 0) {
		truncate(f, newLength);
		fclose(f);
	}
}

void FileUtils::truncate(FILE* f, long newLength) {
#ifdef ES_WINDOWS
	_chsize(_fileno(f), newLength);
#elif ES_GCC
	ftruncate(fileno(f), newLength);
#endif
}

int FileUtils::remove(const string& filePath) {
	return ::remove(filePath.c_str());
}

uint32_t FileUtils::getFileSize(const string& filePath) {
	FILE* file = fopen(filePath.c_str(), "r+b");
	if (file != 0) {
		auto size = getFileSize(file);
		fclose(file);
		return size;
	} else return 0;
}

void FileUtils::createFolder(const string& path) {
#ifdef ES_WINDOWS
	CreateDirectory(path.c_str(), NULL);
#elif ES_GCC
	mkdir(path.c_str(), 0777);
#endif
}

void FileUtils::createFolders(const string& path) {
	vector<string> paths;
	StringUtils::split(path, '/', paths);
	if (paths.size() > 1) {
		string totalPath;
		const string::size_type size = paths.size() - 1;
		for (string::size_type i = 0; i < size; ++i) {
			totalPath += paths[i] + string(FileUtils::PATH_DELIM);
			FileUtils::createFolder(totalPath);
		}
	}
}

#ifdef ES_WINDOWS

void _win32_find_files(const string& path, const string& endsWith, vector<string>& paths) {
	WIN32_FIND_DATA ffd;
	const string pathWithMask = path + string("*");
	HANDLE hFind = FindFirstFile(pathWithMask.c_str(), &ffd);
	if (hFind == INVALID_HANDLE_VALUE) {
		return;
	}

	do {
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			if (strcmp(ffd.cFileName, ".") == 0 || strcmp(ffd.cFileName, "..") == 0)
				continue;

			string newPath = path + string(ffd.cFileName) + string("\\");
			_win32_find_files(newPath, endsWith, paths);
		} else {
			const string fileName(ffd.cFileName);
			if (StringUtils::endsWith(fileName, endsWith)) {
				paths.push_back(path + fileName);
			}
		}

	} while (FindNextFile(hFind, &ffd) != 0);
}

#elif ES_GCC

#include <dirent.h>

void _gcc_find_files(string path, const string& endsWith, vector<string>& paths)
{
	DIR *dir;
	struct dirent *entry;

	if (!(dir = opendir(path.c_str())))
		return;

	while((entry = readdir(dir))) {
		if (entry->d_type == DT_DIR) {
			if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
				continue;
			path += string("/") + string(entry->d_name);
			_gcc_find_files(path, endsWith, paths);
		} else if (entry->d_type == DT_REG) {
			const string fileName(entry->d_name);
			if (StringUtils::endsWith(fileName, endsWith)) {
				paths.push_back(path + string("/") + fileName);
			}
		}
	}

	closedir(dir);
}

#endif


vector<string> FileUtils::findFilesEndingWith(const string& path, const string& suffix) {
	vector<string> paths;
#ifdef ES_WINDOWS
	_win32_find_files(path + PATH_DELIM, suffix, paths);
#elif ES_GCC
	_gcc_find_files(path, suffix, paths);
#endif
	return paths;
}

string FileUtils::getTempDirectory() {
	string pathToTemp;
#ifdef ES_WINDOWS
	char temp[1024] = {0};
	GetTempPath(1024, temp);
	pathToTemp = string(temp) + string("everstore");
#elif ES_GCC
	pathToTemp = string("/tmp/everstore");
#endif
	createFolder(pathToTemp);
	return pathToTemp;
}

string FileUtils::getTempFile() {
	static const int FIRST_CHARACTER = (int) 'a';
	string path = getTempDirectory() + PATH_DELIM;
	for (auto i = 0U; i < 20U; ++i) {
		path += (char) (FIRST_CHARACTER + (rand() % 22));
	}
	return path;
}

string FileUtils::getTempFileName() {
	static const int FIRST_CHARACTER = (int) 'a';
	string path;

	for (auto i = 0U; i < 20U; ++i) {
		path += (char) (FIRST_CHARACTER + (rand() % 22));
	}

	return path;
}

void FileUtils::clearAndDeleteDirectory(const string& path) {
	auto files = FileUtils::findFilesEndingWith(path, string());
	for (auto file : files) {
		::remove(file.c_str());
	}
	::remove(path.c_str());
}

bool FileUtils::copyFile(const string& srcFile, const string& destFile) {
	FILE* file = fopen(srcFile.c_str(), "r+b");
	if (file == NULL) return false;

	const uint32_t fileSize = FileUtils::getFileSize(file);

	char* t = (char*) malloc(fileSize + 1);
	memset(t, 0, fileSize + 1);
	fread(t, fileSize, 1, file);
	fclose(file);

	file = fopen(destFile.c_str(), "a");
	fclose(file);

	file = fopen(destFile.c_str(), "r+b");
	if (file == NULL) {
		free(t);
		return false;
	}

	fwrite(t, fileSize, 1, file);
	fclose(file);
	return true;
}

bool FileUtils::setCurrentDirectory(const string& path) {
#ifdef ES_WINDOWS
	const BOOL okay = SetCurrentDirectory(path.c_str());
	return okay == TRUE;
#elif ES_GCC
	const int okay = chdir(path.c_str());
	return okay == 0;
#endif
}
