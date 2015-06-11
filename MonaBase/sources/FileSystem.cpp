/*
Copyright 2014 Mona
mathieu.poux[a]gmail.com
jammetthomas[a]gmail.com

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License received along this program for more
details (or else see http://www.gnu.org/licenses/).

This file is a part of Mona.
*/


#include "Mona/Util.h"
#include <sys/stat.h>
#include <cctype>
#if defined(_WIN32)
	#include "windows.h"
	#include "direct.h"
#else
    #include "dirent.h"
	#include <unistd.h>
    #include "limits.h"
    #include "pwd.h"
#endif
#include "Mona/FileSystem.h"
#include <set>


namespace Mona {

using namespace std;


#if defined(_WIN32)
typedef struct _stat Status;
#else
typedef struct stat  Status;
#endif


static bool Stat(const string& path, Status& status) {
#if defined (_WIN32)
	wchar_t wFile[_MAX_PATH];
	MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, wFile, _MAX_PATH);
	return _wstat(wFile, &status)==0;
#else
	return ::stat(path.c_str(), &status)==0;
#endif
}


FileSystem::Attributes& FileSystem::GetAttributes(Exception& ex,const char* path,Attributes& attributes) {
	Status status;
	string file(path);
	size_t oldSize(file.size());
	if (!Stat(MakeFile(file), status)) {
		ex.set(Exception::FILE, "Path ", path, " doesn't exist");
		return attributes;
	}
	if (status.st_mode&S_IFDIR) {
		if (oldSize == file.size()) {
			ex.set(Exception::FILE, "File ", path, " doesn't exist");
			return attributes;
		}
	} else if (oldSize > file.size()) {
		ex.set(Exception::FILE, "Folder ", path, " doesn't exist");
		return attributes;
	}
	attributes.lastModified.update(status.st_mtime*1000ll);
	attributes.size = oldSize > file.size() ? 0 : (UInt32)status.st_size;
	return attributes;
}

bool FileSystem::Exists(const char* path) {
	Status status;
	string file(path);
	size_t oldSize(file.size());
	if (!Stat(MakeFile(file), status))
		return false;
	// if existing test was on folder
	return status.st_mode&S_IFDIR ? oldSize>file.size() : oldSize==file.size();
}

void FileSystem::CreateDirectories(Exception& ex,const string& path) {
	vector<string> directories;
	if (Unpack(path, directories).empty())
		return;
	string dir;
	if (directories.front().size()==2 && directories.front().back() == ':') {
		// device
		dir.append(directories.front());
		directories.erase(directories.begin());
	}
	for (const string& directory : directories) {
		dir.append("/");
		dir.append(directory);
		if (!CreateDirectory(dir)) {
			ex.set(Exception::FILE, "Impossible to create ", dir, " directory");
			return;
		}
	}
}


bool FileSystem::CreateDirectory(const char* path) {
	Status status;
	string file(path);
	if (Stat(MakeFile(file), status)) // exists already
		return status.st_mode&S_IFDIR ? true : false;
#if defined(_WIN32)
	wchar_t wFile[_MAX_PATH];
	MultiByteToWideChar(CP_UTF8, 0, file.c_str(), -1, wFile, _MAX_PATH);
	return _wmkdir(wFile) == 0;
#else
    return mkdir(file.c_str(),S_IRWXU | S_IRWXG | S_IRWXO) == 0;
#endif
}

bool FileSystem::Remove(Exception& ex,const char* path,bool all) {
	bool isFolder(IsFolder(path));
	if (all && isFolder) {
		FileSystem::ForEach forEach([&ex](const string& filePath){
			Remove(ex, filePath, true);
		});
		Exception ignore;
		Paths(ignore, path, forEach); // if exception it's a not existent folder
	}
	if (!Exists(path))
		return !ex;; // already removed!
#if defined(_WIN32)
	if (isFolder) {
		wchar_t wFile[_MAX_PATH];
		MultiByteToWideChar(CP_UTF8, 0, path, -1, wFile, _MAX_PATH);
		if (RemoveDirectoryW(wFile)==0)
			ex.set(Exception::FILE, "Impossible to remove folder ", path);
	} else {
		wchar_t wFile[_MAX_PATH];
		MultiByteToWideChar(CP_UTF8, 0, path, -1, wFile, _MAX_PATH);
		if(_wremove(wFile) != 0)
			ex.set(Exception::FILE, "Impossible to remove file ", path);
	}
	return !ex;
#endif
	if (remove(path) == 0)
		return !ex;
	if (isFolder)
		ex.set(Exception::FILE, "Impossible to remove folder ", path);
	else
		ex.set(Exception::FILE, "Impossible to remove file ", path);
	return false;
}

UInt32 FileSystem::Paths(Exception& ex, const char* path, const ForEach& forEach) {
	int err = 0;
	string directory(path);
	FileSystem::MakeDirectory(directory);
	UInt32 count(0);
	string pathFile;

#if defined(_WIN32)
	directory.append("*");

	wchar_t wDirectory[_MAX_PATH];
	MultiByteToWideChar(CP_UTF8, 0, directory.c_str(), -1, wDirectory, _MAX_PATH);
	
	WIN32_FIND_DATAW fileData;
	HANDLE	fileHandle = FindFirstFileW(wDirectory, &fileData);
	if (fileHandle == INVALID_HANDLE_VALUE) {
		if ((err = GetLastError()) != ERROR_NO_MORE_FILES) {
			ex.set(Exception::FILE, "The system cannot find the directory ", path);
			return count;
		}
		return count;
	}
	do {
		if (wcscmp(fileData.cFileName, L".") != 0 && wcscmp(fileData.cFileName, L"..") != 0) {
			++count;
			String::Append(MakeDirectory(pathFile.assign(path)), fileData.cFileName);
			if (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				pathFile.append("/");
			forEach(pathFile);
		}
	} while (FindNextFileW(fileHandle, &fileData) != 0);
	FindClose(fileHandle);
#else
	DIR* pDirectory = opendir(directory.c_str());
	if (!pDirectory) {
		ex.set(Exception::FILE, "The system cannot find the directory ",directory);
		return count;
	}
	struct dirent* pEntry(NULL);
	while((pEntry = readdir(pDirectory))) {
		if (strcmp(pEntry->d_name, ".")!=0 && strcmp(pEntry->d_name, "..")!=0) {
			++count;
			String::Append(pathFile.assign(directory), pEntry->d_name);
			// Cross-platform solution when DT_UNKNOWN or symbolic link
			if(pEntry->d_type==DT_DIR)
				pathFile.append("/");
			else if(pEntry->d_type==DT_UNKNOWN || pEntry->d_type==DT_LNK) {
				Status status;
				Stat(pathFile, status);
				if ((status.st_mode&S_IFMT) == S_IFDIR)
					pathFile.append("/");
			}
			forEach(pathFile);
		}
	}
	closedir(pDirectory);
#endif
	return count;
}

Time& FileSystem::GetLastModified(Exception& ex, const char* path, Time& time) {
	Status status;
	status.st_mtime = time/1000;
	string file(path);
	size_t oldSize(file.size());
	if (Stat(MakeFile(file).c_str(), status) != 0) {
		ex.set(Exception::FILE, "Path ", path, " doesn't exist");
		return time;
	}
	
	if (status.st_mode&S_IFDIR) {
		 // if path is file
		if (oldSize==file.size()) {
			ex.set(Exception::FILE, "File ", path, " doesn't exist");
			return time;
		}
	} else if (oldSize>file.size()) {
		 // if path is folder
		ex.set(Exception::FILE, "Folder ", path, " doesn't exist");
		return time;
	}
	time.update(status.st_mtime*1000ll);
	return time;
}

UInt32 FileSystem::GetSize(Exception& ex,const char* path) {
	Status status;
	status.st_size = 0;
	string file(path);
	size_t oldSize(file.size());
	if (Stat(MakeFile(file).c_str(), status) != 0 || status.st_mode&S_IFDIR) {
		ex.set(Exception::FILE, "File ", path, " doesn't exist");
		return 0;
	} else if (oldSize>file.size()) { // if was a folder
		ex.set(Exception::FILE, "GetSize works just on file, and ", path, " is a folder");
		return 0;
	}
	return (UInt32)status.st_size;
}

string& FileSystem::GetName(string& path) {
	MakeFile(path);
	auto separator = path.find_last_of("/\\");
	if (separator != string::npos)
		path.erase(0, separator + 1);
	return path;
}


string& FileSystem::GetBaseName(string& path) {
	MakeFile(path);
	auto dot = path.find_last_of('.');
	auto separator = path.find_last_of("/\\");
	if (dot != string::npos && (separator == string::npos || dot > separator))
		path.resize(dot);
	if (separator != string::npos)
		path.erase(0, separator + 1);
	return path;
}


string& FileSystem::GetExtension(string& path) {
	MakeFile(path);
	auto dot = path.find_last_of('.');
	auto separator = path.find_last_of("/\\");
	if (dot != string::npos && (separator == string::npos || dot > separator))
		return path.erase(0, dot + 1);
	path.clear();
	return path;
}

string& FileSystem::MakeFile(string& path) {
	size_t size = path.size();
	while (size>0 && (path.back() == '\\' || path.back() == '/'))
		path.resize(--size);
	return path;
}

string& FileSystem::GetParent(string& path) {
	auto separator = MakeFile(path).find_last_of("/\\");
	if (separator != string::npos)
		path.erase(separator+1); // keep the "/" (= folder!)
	else
		path.assign(".");
	return path;
}

string& FileSystem::Pack(const vector<string>& values, string& path) {
	path.clear();
	bool first = true;
	for (const string& value : values) {
		if (value.empty())
			continue;
#if defined(_WIN32)
		if (first) {
			if (value.size()!=2 || value.back()!= ':') // device case
				path.append("/");
			first = false;
		} else
			path.append("/");
#else
		path.append("/");
#endif
		path.append(value);
	}
	return path;
}

vector<string>& FileSystem::Unpack(const char* path, vector<string>& values) {
	const char* it(path);
	const char* itValue;

	while (*it) {

		// trim begin
		while (isspace(*it))
			++it;

		itValue = it;
		bool first = true;
		while (*it && *it != '\\' && *it != '/') {
			int count(0);
			// resolve '..' and '.'
			if (first) {
				while (*it == '.') {
					++count;
					++it;
				}
				first = false;
				if (count > 0 && *it != '\\' && *it != '/')
					continue;
			}
			if (count == 0)
				++it;
			else {
				itValue = it;
				if (count > 1) {
					if (!values.empty())
						values.pop_back();
				}
			}
		}

		// trim end
		if (it != itValue) {
			while (--it != itValue && isspace(*it));
			++it;
			values.emplace_back(itValue, it);
		}
		if (*it)
			++it;
	}
	return values;
}

// TODO test unit
bool FileSystem::GetHome(string& path) {
#if defined(_WIN32)
	// windows

	// windows service has no home dir, return system directory instead
	string homePath;
	if (!Util::Environment().getString("HOMEDRIVE", path) || !Util::Environment().getString("HOMEPATH", homePath)) {
		// system directory
		char buffer[MAX_PATH];
		DWORD n = GetSystemDirectoryA(buffer, sizeof(buffer));
		if (n <= 0 || n >= sizeof(buffer))
			return false;
		path.assign(buffer);
	}
	path.append(homePath);
#else
	struct passwd* pwd = getpwuid(getuid());
	if (pwd)
		path.assign(pwd->pw_dir);
	else {
		pwd = getpwuid(geteuid());
		if (pwd)
			path.assign(pwd->pw_dir);
		else if(!Util::Environment().getString("HOME", path))
			return false;
	}
#endif
	MakeDirectory(path);
	return true;
}

bool FileSystem::IsFolder(const string& path) {
	if (path.empty())
		return false;
	char last(path.back());
	if (last == '/' || last == '\\')
		return true;
#if defined(_WIN32)
	return path.size()==2 && last == ':';
#else
	return path.size()==1 && last == '~';
#endif
}

bool FileSystem::IsFolder(const char* path) {
	size_t size(strlen(path));
	if (size == 0)
		return false;
	char last(path[--size]);
	if (last == '/' || last == '\\')
		return true;
#if defined(_WIN32)
	return size==2 && last == ':';
#else
	return size==1 && last == '~';
#endif
}


bool FileSystem::IsAbsolute(const string& path) {
	if (path.empty())
		return false;
#if defined(_WIN32)
	return path.size()>1 && isalpha(path[0]) && path[1]==':';
#else
	if (path[0] == '/')
		return true;
	if (path[0] != '~')
		return false;
	return path.size()==1 || path[1] == '/'; // everything in the form ~/... is absolute
#endif
}
bool FileSystem::IsAbsolute(const char* path) {
	if (*path==0) // strlen(path)==0
		return false;
#if defined(_WIN32)
	return path[1] && isalpha(path[0]) && path[1]==':';
#else
	if (*path == '/')
		return true;
	if (*path != '~')
		return false;
	return *++path==0 || *path == '/'; // everything in the form ~/... is absolute
#endif
}

bool FileSystem::ResolveFileWithPaths(const char* paths, string& file) {


	String::ForEach forEach([&file](UInt32 index,const char* value) {
		string path(value);
		#if defined(_WIN32)
			// "path" => path
			if (!path.empty() && path[0] == '"' && path.back() == '"') {
				path.resize(path.size()-1);
				path.erase(0, 1);
			}
		#endif
		MakeDirectory(path).append(file);
		if (Exists(path)) {
			file = move(path);
			return false;
		}
		return true;
	});
#if defined(_WIN32)
	return String::Split(paths, ";", forEach, String::SPLIT_IGNORE_EMPTY | String::SPLIT_TRIM) == string::npos;
#else
	return String::Split(paths, ":", forEach, String::SPLIT_IGNORE_EMPTY | String::SPLIT_TRIM) == string::npos;
#endif
}

bool FileSystem::GetCurrentApplication(string& path) {
	string result;
#ifdef _WIN32
	result.resize(MAX_PATH);
	int n = GetModuleFileNameA(0, &result[0], MAX_PATH);
	if (n <= 0)
		return false;
	result.resize(n);
#else
	result.resize(130);
	
	// read the link target into variable linkTarget
	ssize_t n(130); 

	while(n>=result.size()) {
		result.resize(result.size()*2);
		if((n = readlink("/proc/self/exe", &result[0], result.size()))<=0)
			return false;
	}
	result.resize(n);
#endif
	path = move(result);
	return true;
}

bool FileSystem::GetCurrent(string& path) {
	string::size_type size = path.size();
#if defined(_WIN32)
	path.resize(MAX_PATH);
	int len = GetCurrentDirectoryA(MAX_PATH, &path[0]);
	if (len <= 0) {
		path.resize(size);
		return false;
	}
	path.resize(len);
#else
	path.resize(PATH_MAX);
	if (!getcwd(&path[0], PATH_MAX)) {
		path.resize(size);
		return false;
	}
	path.resize(strlen(path.c_str()));
#endif
	MakeDirectory(path);
	return true;
}

} // namespace Mona
