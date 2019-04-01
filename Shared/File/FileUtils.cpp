#include "FileUtils.h"

using namespace std;

const char FileUtils::EMPTY = '\0';
const char FileUtils::SPACE = ' ';
const int FileUtils::SPACE_SIZE = 1;
const char FileUtils::NL = '\n';
const int FileUtils::NL_SIZE = 1;

#ifdef WIN32

const char FileUtils::PATH_DELIM = '\\';

#else

const char FileUtils::PATH_DELIM = '/';

#endif

#ifdef WIN32

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <io.h>

#else

#include <sys/stat.h>

#endif

bool FileUtils::truncate(const string& fileName, long newLength) {
	FILE* f = fopen(fileName.c_str(), "r+b");
	if (f) {
		const auto result = truncate(f, newLength);
		fclose(f);
		return result;
	}
	return false;
}

bool FileUtils::truncate(FILE* f, long newLength) {
#ifdef WIN32
	return _chsize(_fileno(f), newLength) == 0;
#else
	return ftruncate(fileno(f), newLength) == 0;
#endif
}

void FileUtils::createFolder(const string& path) {
#ifdef WIN32
	CreateDirectory(path.c_str(), NULL);
#else
	mkdir(path.c_str(), 0777);
#endif
}

#ifdef WIN32

void _win32_find_files(const string& path, const string& endsWith, vector<string>& paths) {
	WIN32_FIND_DATA ffd;
	const string pathWithMask = path + string("*");
	HANDLE hFind = FindFirstFile(pathWithMask.c_str(), &ffd);
	if (hFind == INVALID_HANDLE_VALUE)  {
		return;
	}

	do {
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			if (strcmp(ffd.cFileName, ".") == 0 || strcmp(ffd.cFileName, "..") == 0)
				continue;

			string newPath = path + string(ffd.cFileName) + string("\\");
			_win32_find_files(newPath, endsWith, paths);
		}
		else {
			const string fileName(ffd.cFileName);
			if (StringUtils::endsWith(fileName, endsWith)) {
				paths.push_back(path + fileName);
			}
		}

	} while (FindNextFile(hFind, &ffd) != 0);
}

#else

#include <dirent.h>

void _gcc_find_files(string path, const string& endsWith, vector<string>& paths) {
	DIR* dir;
	struct dirent* entry;

	if (!(dir = opendir(path.c_str())))
		return;

	while ((entry = readdir(dir))) {
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


vector<string> FileUtils::findFilesEndingWith(const string& path, const string& sufix) {
	vector<string> paths;
#ifdef WIN32
	_win32_find_files(path + string("\\"), sufix, paths);
#else
	_gcc_find_files(path, sufix, paths);
#endif
	return paths;
}

string FileUtils::getTempDirectory() {
	string pathToTemp;
#ifdef WIN32
	char temp[1024] = { 0 };
	GetTempPath(1024, temp);
	pathToTemp = string(temp) + string("everstore");
#else
	pathToTemp = string("/tmp/everstore");
#endif
	createFolder(pathToTemp);
	return pathToTemp;
}

string FileUtils::getTempFile() {
	static constexpr int firstChar = (int) 'a';
	string path = getTempDirectory() + string(1, FileUtils::PATH_DELIM);
	for (auto i = 0U; i < 20U; ++i) {
		path += (char) (firstChar + (rand() % 22));
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

bool FileUtils::copyFile(const Path& srcFile, const Path& destFile) {
	auto file = srcFile.Open();
	if (file == nullptr) return false;

	const uint32_t fileSize = FileUtils::getFileSize(file);

	char* t = (char*) malloc(fileSize + 1);
	memset(t, 0, fileSize + 1);
	fread(t, fileSize, 1, file);
	fclose(file);

	file = destFile.OpenOrCreate("r+b");
	if (file == NULL) {
		free(t);
		return false;
	}

	fwrite(t, fileSize, 1, file);
	fclose(file);
	return true;
}

bool FileUtils::setCurrentDirectory(const string& path) {
#ifdef WIN32
	const BOOL okay = SetCurrentDirectory(path.c_str());
	return okay == TRUE;
#else
	const int okay = chdir(path.c_str());
	return okay == 0;
#endif
}
