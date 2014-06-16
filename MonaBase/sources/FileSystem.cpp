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

// TODO eliminate everywhere the VMS support!
#if defined(_WIN32)
const string FileSystem::_PathSeparator(";");
#else
const string FileSystem::_PathSeparator(":");
#endif

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


FileSystem::Attributes& FileSystem::GetAttributes(Exception& ex,const string& path,Attributes& attributes) {

	Status status;
	status.st_mtime = attributes.lastModified/1000;
	status.st_size = attributes.size;

	string file(path);
	if (!Stat(MakeFile(file), status)) {
		ex.set(Exception::FILE, "Path ", path, " doesn't exist");
		return attributes;
	}
	attributes.lastModified.update(status.st_mtime*1000ll);
	if(!(attributes.isDirectory = (status.st_mode&S_IFDIR) ? true : false))
		attributes.size = (UInt32)status.st_size;
	return attributes;
}

bool FileSystem::Exists(const string& path, bool any) {
	Status status;
	string file(path);
	if (!Stat(MakeFile(file), status))
		return false;
	if (any)
		return true;
	// if existing test was on folder
	if (path.size() > file.size())
		return status.st_mode&S_IFDIR ? true : false;
	return status.st_mode&S_IFDIR ? false : true;
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


bool FileSystem::CreateDirectory(const string& path) {
	Status status;
	string file(path);
	if (Stat(MakeFile(file), status)) // exists already
		return status.st_mode&S_IFDIR ? true : false;
#if defined(_WIN32)
	wchar_t wFile[_MAX_PATH];
	MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, wFile, _MAX_PATH);
	return _wmkdir(wFile) == 0;
#else
    return mkdir(path.c_str(),S_IRWXU | S_IRWXG | S_IRWXO) == 0;
#endif
}

bool FileSystem::Remove(Exception& ex,const string& path,bool all) {
	if (all) {
		FileSystem::ForEach forEach([&ex](const string& filePath){
			Remove(ex, filePath, true);
		});
		Exception ignore;
		Paths(ignore, path, forEach); // if exception it's a file or a not existent folder
	}

	if (!Exists(path, true))
		return !ex;
	if (remove(path.c_str()) == 0)
		return !ex;
#if defined(_WIN32)
	wchar_t wFile[_MAX_PATH];
	MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, wFile, _MAX_PATH);
	if (RemoveDirectoryW(wFile) != 0)
		return !ex;
#endif
	ex.set(Exception::FILE, "Impossible to remove ", path);
	return false;
}

UInt32 FileSystem::Paths(Exception& ex, const string& path, const FileSystem::ForEach& forEach) {
	int err = 0;
	string directory(path);
	FileSystem::MakeDirectory(directory);
	UInt32 count(0);
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
			char file[_MAX_FNAME];
			WideCharToMultiByte(CP_UTF8, 0, fileData.cFileName, -1, file, _MAX_FNAME, NULL, NULL);
			string pathFile(path);
			++count;
			forEach(String::Append(MakeDirectory(pathFile), file));
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
	while(pEntry = readdir(pDirectory)) {
		if (strcmp(pEntry->d_name, ".")!=0 && strcmp(pEntry->d_name, "..")!=0) {
			string pathFile(directory);
			++count;
            forEach(String::Append(pathFile, pEntry->d_name));
		}
	}
	closedir(pDirectory);
#endif
	return count;
}

Time& FileSystem::GetLastModified(Exception& ex, const string& path, Time& time) {
	Status status;
	status.st_mtime = time/1000;
	string file(path);
	if (Stat(MakeFile(file).c_str(), status) != 0)
		ex.set(Exception::FILE, "Path ", path, " doesn't exist");
	else
		time.update(status.st_mtime*1000ll);
	return time;
}

UInt32 FileSystem::GetSize(Exception& ex,const string& path) {
	Status status;
	status.st_size = 0;
	string file(path);
	if (Stat(MakeFile(file).c_str(), status) != 0)
		ex.set(Exception::FILE, "File ", path, " doesn't exist");
	else if (status.st_mode&S_IFDIR) // folder, no GetSize possible
		ex.set(Exception::FILE, "GetSize works just on file, and ", path, " is a folder");
	if (ex)
		status.st_size = 0; // Cause on linux, for a folder, it's equal to 4096...
	return (UInt32)status.st_size;
}

string& FileSystem::GetName(const string& path, string& value) {
	value.assign(path);
	auto separator = value.find_last_of("/\\");
	if (separator != string::npos)
		value.erase(0, separator + 1);
	return value;
}

string& FileSystem::GetBaseName(const string& path, string& value) {
	value.assign(path);
	auto dot = value.find_last_of('.');
	auto separator = value.find_last_of("/\\");
	if (dot != string::npos && (separator == string::npos || dot > separator))
		value.resize(dot);
	if (separator != string::npos)
		value.erase(0, separator + 1);
	return value;
}


string& FileSystem::GetExtension(const string& path, string& value) {
	value.assign(path);
	auto dot = value.find_last_of('.');
	auto separator = value.find_last_of("/\\");
	if (dot != string::npos && (separator == string::npos || dot > separator))
		value.erase(0, dot + 1);
	else
		value.clear();
	return value;
}

string& FileSystem::MakeFile(string& path) {
	string::size_type size = path.size();
#if defined(_WIN32)
	while (size>0 && (path.back() == '\\' || path.back() == '/'))
		path.resize(--size);
#else
	while (size>0 && path.back() == '/')
		path.resize(--size);
#endif
	return path;
}

string& FileSystem::GetParent(string& path) {
	auto separator = MakeFile(path).find_last_of("/\\");
	if (separator != string::npos)
		path.erase(separator+1); // keep the "/" (= folder!)
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

vector<string>& FileSystem::Unpack(const string& path, vector<string>& values) {
	string::const_iterator it = path.begin(), itValue, end = path.end();

	while (it != end) {

		// trim begin
		while (isspace(*it))
			++it;

		itValue = it;
		bool first = true;
		while (it != end && *it != '\\' && *it != '/') {
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
		if (it != end)
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

// TODO test unit
bool FileSystem::IsAbsolute(const string& path) {
#if defined(_WIN32)
	return !path.empty() && isalpha(path[0]) && (path.size()!=2 || path.back()==':');
#else
	if (path.empty())
		return false;
	if (path[0] == '/')
		return true;
	if (path[0] != '~')
		return false;
	return path.size()<2 || path[1] == '/';
#endif
}

bool FileSystem::ResolveFileWithPaths(const string& paths, string& file) {

	vector<string> values;
	String::Split(paths, _PathSeparator, values, String::SPLIT_IGNORE_EMPTY | String::SPLIT_TRIM);

	for (string& value : values) {
#if defined(WIN32)
		// "path" => path
		if (value.size() > 1 && value[0] == '"' && value.back() == '"') {
			value.resize(value.size()-1);
			value.erase(0, 1);
		}
#endif
		MakeDirectory(value);
		value += file;
		if (Exists(value)) {
			file = move(value);
			return true;
		}
	}
	return false;
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
