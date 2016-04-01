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


namespace Mona {

using namespace std;


FileSystem::Home::Home() {
#if defined(_WIN32)
	// windows

	const char* path(Util::Environment().getRaw("HOMEPATH"));
	if (path) {
		if (!Util::Environment().getString("HOMEDRIVE", *this) && !Util::Environment().getString("SystemDrive", *this))
			assign("C:");
		append(path);
	} else if (!Util::Environment().getString("USERPROFILE", *this)) {
		clear();
		return;
	}
#else
	struct passwd* pwd = getpwuid(getuid());
	if (pwd)
		assign(pwd->pw_dir);
	else {
		pwd = getpwuid(geteuid());
		if (pwd)
			assign(pwd->pw_dir);
		else if(!Util::Environment().getString("HOME", *this)) {
			clear();
			return;
		}
	}
#endif
	
	MakeFolder(*this);
}


FileSystem::CurrentApp::CurrentApp() {
	resize(PATH_MAX);
#ifdef _WIN32
	int n = GetModuleFileNameA(0, &(*this)[0], PATH_MAX);
	if (n <= 0 || n > PATH_MAX) {
		clear();
		return;
	}
#else
	// read the link target into variable linkTarget
	ssize_t n = readlink("/proc/self/exe", &(*this)[0], PATH_MAX); 
	if(n<=0) {
		clear();
		return;
	}
#endif
	resize(n);
}



FileSystem::CurrentDirs::CurrentDirs() : _isNull(false) {
	string current;
	int n(0);
	current.resize(PATH_MAX);
#if defined(_WIN32)
	char* test(&current[0]);
	n = GetCurrentDirectoryA(PATH_MAX, test);
	if (n < 0 || n > PATH_MAX)
		n = 0;
#else
	if (getcwd(&current[0], PATH_MAX)) {
		n = strlen(current.c_str());
		emplace_back("/");
	}
#endif
	current.resize(n);
	_isNull = n == 0;
	if (_isNull) {
		// try with parrent of CurrentApp
		if (_CurrentApp)
			GetParent(_CurrentApp, current);
		else // else with Home
			current = _Home;
	}

	String::ForEach forEach([this](UInt32 index,const char* value){
		Directory dir(empty() ? String::Empty : back());
		dir.append(value).append("/");
		dir.name.assign(value);
		dir.extPos = dir.name.find_last_of('.');
		emplace_back(move(dir));
		return true;
	});
	String::Split(current, "/\\", forEach, String::SPLIT_IGNORE_EMPTY);
	if (empty())
		emplace_back("/"); // if nothing other possible, root!
}


#if defined(_WIN32)
typedef struct _stat Status;
#else
typedef struct stat  Status;
#endif


static Int8 Stat(const char* path, size_t size, Status& status) {

	bool isFolder;
	if (size) {
		char c(path[size-1]);
		isFolder = (c=='/' || c=='\\');
	} else
		isFolder = true;

	bool found;

#if defined (_WIN32)
	// windows doesn't accept blackslash in _wstat
	wchar_t wFile[_MAX_PATH];
	size = MultiByteToWideChar(CP_UTF8, 0, path, size, wFile, _MAX_PATH);
	if (isFolder)
		wFile[size++] = '.';
	wFile[size] = 0;
	found = _wstat(wFile, &status)==0;
#else
	found = ::stat(size ? path : ".", &status)==0;
#endif
	if (!found)
		return 0;
	return status.st_mode&S_IFDIR ? (isFolder ? 1 : -1) : (isFolder ? -1 : 1);
}


FileSystem::Attributes& FileSystem::GetAttributes(const char* path,size_t size, Attributes& attributes) {
	Status status;
	if (Stat(path, size, status) <= 0)
		return attributes.reset();
	attributes.lastModified.update(status.st_mtime*1000ll);
	attributes.size = status.st_mode&S_IFDIR ? 0 : (UInt32)status.st_size;
	return attributes;
}

Time& FileSystem::GetLastModified(Exception& ex, const char* path, size_t size, Time& time) {
	Status status;
	status.st_mtime = time/1000;
	if (Stat(path, size, status)<=0) {
		ex.set(Exception::FILE, path, " doesn't exist");
		return time;
	}
	time.update(status.st_mtime*1000ll);
	return time;
}

UInt32 FileSystem::GetSize(Exception& ex,const char* path, size_t size, UInt32 defaultValue) {
	Status status;
	if (Stat(path, size, status)<=0) {
		ex.set(Exception::FILE, path, " doesn't exist");
		return defaultValue;
	}
	if (status.st_mode&S_IFDIR) { // if was a folder
		ex.set(Exception::FILE, "GetSize works just on file, and ", path, " is a folder");
		return defaultValue;
	}
	return (UInt32)status.st_size;
}

bool FileSystem::Exists(const char* path, size_t size) {
	Status status;
	return Stat(path, size, status)>0;
}


bool FileSystem::CreateDirectory(Exception& ex, const char* path, size_t size, Mode mode) {
	Status status;
	if (Stat(path, size, status)) {
		if (status.st_mode&S_IFDIR)
			return true;
		ex.set(Exception::FILE,"Cannot create directory ",path," because a file with this path exists");
		return false;
	}

	if (mode==HEAVY) {
		// try to create the parent (recursive)
		string parent;
		GetParent(path,parent);
		if (parent.compare(path) && !CreateDirectory(ex, parent, HEAVY))
			return false;
	}

#if defined(_WIN32)
	wchar_t wFile[_MAX_PATH];
	MultiByteToWideChar(CP_UTF8, 0, path, -1, wFile, _MAX_PATH);
	if (_wmkdir(wFile) == 0)
		return true;
#else
    if (mkdir(path,S_IRWXU | S_IRWXG | S_IRWXO)==0)
		return true;
#endif
	ex.set(Exception::FILE,"Cannot create directory ",path);
	return false;
}

bool FileSystem::Delete(Exception& ex, const char* path, size_t size, Mode mode) {
	Status status;
	if (!Stat(path, size, status))
		return true; // already deleted

	if (status.st_mode&S_IFDIR) {
		if (!size)
			path = ".";
		if (mode==HEAVY) {
			FileSystem::ForEach forEach([&ex](const string& path, UInt16 level) {
				Delete(ex, path, HEAVY);
			});
			Exception ignore;
			ListFiles(ignore, path, forEach);
			if (ignore)
				return true;; // if exception it's a not existent folder
			if (ex) // impossible to remove a sub file/folder, so the parent folder can't be removed too (keep the exact exception)
				return false;
		}
	}
#if defined(_WIN32)
	wchar_t wFile[_MAX_PATH];
	MultiByteToWideChar(CP_UTF8, 0, path, -1, wFile, _MAX_PATH);
	if (status.st_mode&S_IFDIR) {
		if (RemoveDirectoryW(wFile) || GetLastError() == ERROR_FILE_NOT_FOUND)
			return true;
		ex.set(Exception::FILE, "Impossible to remove folder ", path);
	} else {
		if (DeleteFileW(wFile) || GetLastError()==ERROR_FILE_NOT_FOUND)
			return true;
		ex.set(Exception::FILE, "Impossible to remove file ", path);
	}
#else
	if(status.st_mode&S_IFDIR) {
		if(rmdir(path)==0 || errno==ENOENT)
			return true;
		ex.set(Exception::FILE, "Impossible to remove folder ", path);
	} else {
		if(unlink(path)==0 || errno==ENOENT)
			return true;
		ex.set(Exception::FILE, "Impossible to remove file ", path);
	}
#endif
	return false;
}

UInt32 FileSystem::ListFiles(Exception& ex, const char* path, const ForEach& forEach, Mode mode) {
	string directory(*path ? path : ".");
	MakeFolder(directory);
	UInt32 count(0);
	string file;

#if defined(_WIN32)
	wchar_t wDirectory[_MAX_PATH];
	wDirectory[MultiByteToWideChar(CP_UTF8, 0, directory.data(), directory.size(), wDirectory, _MAX_PATH)] = '*';
	wDirectory[directory.size()+1] = 0;
	
	WIN32_FIND_DATAW fileData;
	HANDLE	fileHandle = FindFirstFileW(wDirectory, &fileData);
	if (fileHandle == INVALID_HANDLE_VALUE) {
		if (GetLastError() != ERROR_NO_MORE_FILES)
			ex.set(Exception::FILE, "Cannot list files of directory ", directory);
		return 0;
	}
	do {
		if (wcscmp(fileData.cFileName, L".") != 0 && wcscmp(fileData.cFileName, L"..") != 0) {
			++count;
			String::Format(file,directory, fileData.cFileName);
			if (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				file += '/';
				if (mode)
					count += ListFiles(ex, file, forEach, Mode(mode+1));
			}
			forEach(file,mode ? (UInt16(mode)-1) : 0);
		}
	} while (FindNextFileW(fileHandle, &fileData) != 0);
	FindClose(fileHandle);
#else
	DIR* pDirectory = opendir(directory.c_str());
	if (!pDirectory) {
		ex.set(Exception::FILE, "Cannot list files of directory ",directory);
		return 0;
	}
	struct dirent* pEntry(NULL);
	while((pEntry = readdir(pDirectory))) {
		if (strcmp(pEntry->d_name, ".")!=0 && strcmp(pEntry->d_name, "..")!=0) {
			++count;
			String::Format(file, directory, pEntry->d_name);
			// Cross-platform solution when DT_UNKNOWN or symbolic link
			bool isFolder(false);
			if(pEntry->d_type==DT_DIR) {
				isFolder = true;
			} else if(pEntry->d_type==DT_UNKNOWN || pEntry->d_type==DT_LNK) {
				Status status;
				Stat(file.data(), file.size(), status);
				if ((status.st_mode&S_IFMT) == S_IFDIR)
					isFolder = true;
			}
			if(isFolder) {
				file += '/';
				if (mode)
					count += ListFiles(ex, file, forEach, Mode(mode+1));
			}
			forEach(file,mode ? (UInt16(mode)-1) : 0);
		}
	}
	closedir(pDirectory);
#endif
	return count;
}

const char* FileSystem::GetFile(const char* path, size_t& size, size_t& extPos, Type& type, Int32& parentPos) {
	const char* cur(path + size);
	size = 0;
	bool   scanDots(true);
	UInt16 level(1);
	extPos = string::npos;
	bool firstChar(true);
	type = FOLDER;

	while (cur-- > path) {
		if (*cur == '/' || *cur == '\\') {
			if (firstChar) {
				type = FOLDER;
				firstChar = false;
			}
			if (size) {
				if (scanDots)
					level += UInt16(size);
				else {
					if (!level)
						break;
					scanDots = true;
					extPos = string::npos;
				}
				size = 0;
			}
			continue;
		}

		if (firstChar) {
			type = FILE;
			firstChar = false;
		}

		if (!scanDots && level)
			continue;
		if (!size++)
			--level;

		if (extPos==string::npos && *cur == '.') {
			if (scanDots) {
				if (size < 3)
					continue;
				extPos = 1;
			} else
				extPos = size;
		}
		scanDots = false;
	}

	if (scanDots) // nothing or . or .. or something with backslash (...../)
		level += UInt16(size);

	if (level) {
		size = 0; // no name!
		if (FileSystem::IsAbsolute(path)) {
#if defined(_WIN32)
			parentPos = isalpha(*path) ? 3 : 1; // C:/ or /
#else
			parentPos = 1; // /
#endif	
			return path+parentPos;
		}
		
		parentPos = -level;
		return NULL; // level!
	}
#if defined(_WIN32)
	else if (cur < path && size == 2 && type==FOLDER && path[1] == ':' && isalpha(*path)) {
		parentPos = 3; // C:/
		size = 0; // no name!
		return path+3;
	}
#endif
	
	const char* name(++cur);
	if (extPos!=string::npos)
		extPos = size - extPos;	

	UInt8 offset(1);
	while (--cur >= path && (*cur == '/' || *cur == '\\'))
		offset = 2;

	parentPos = cur - path + offset;
	return name;
}


FileSystem::Type FileSystem::GetFile(const char* path, size_t size, string& name, size_t& extPos, string& parent) {
	
	Type type;
	Int32 parentPos;
	const char* file(GetFile(path, size, extPos, type, parentPos));
	if (file)
		name.assign(file,size);
	else
		name.clear();

	if (parentPos <= 0) {
		// parent is -level!
		parentPos += _CurrentDirs.size()-1;
		if (parentPos<0)
			parentPos = 0;
		if (&parent!=&String::Empty)
			parent.assign(_CurrentDirs[parentPos]);
		if (++parentPos<_CurrentDirs.size()) {
			Directory& dir(_CurrentDirs[parentPos]);
			name.assign(dir.name);
			extPos = dir.extPos;
		}
	} else if (&parent!=&String::Empty)
		parent.assign(path, 0, parentPos);

	return type;
}


string& FileSystem::GetName(const char* path, string& value) {
	GetFile(path,value);
	return value;
}

string& FileSystem::GetBaseName(const char* path, string& value) {
	size_t extPos;
	GetFile(path,value,extPos);
	if (extPos!=string::npos)
		value.erase(extPos);
	return value;
}

string& FileSystem::GetExtension(const char* path, string& value) {
	size_t extPos;
	GetFile(path,value,extPos);
	if (extPos == string::npos)
		value.clear();
	else
		value.erase(0, extPos+1);
	return value;
}

string& FileSystem::GetParent(const char* path, size_t size, string& value) {
	Type type;
	Int32 parentPos;
	size_t extPos;
	GetFile(path, size, extPos, type, parentPos);
	if (parentPos > 0)
		return value.assign(path, 0, parentPos);
	parentPos += _CurrentDirs.size()-1;
	return value.assign(_CurrentDirs[parentPos<0 ? 0 : parentPos]);
}

string& FileSystem::Resolve(string& path) {
	Type type(FOLDER);
	size_t extPos;
	string newPath;
	size_t size;
	Int32 parentPos;
	const char* file;

	do {
		if (type == FILE)
			path += '.';
		file = GetFile(path.data(),size = path.size(), extPos, type,parentPos);

		if (parentPos <= 0) {
			parentPos += _CurrentDirs.size()-1;
			if (parentPos<0)
				parentPos = 0;
			if (++parentPos < _CurrentDirs.size()) {
				newPath.insert(0, _CurrentDirs[parentPos]);
				if (type == FILE)
					MakeFile(newPath);
			} else {
				newPath.insert(0, file, size).insert(0, _CurrentDirs[--parentPos]);
				if (type == FOLDER)
					newPath += '/';
			}
			return path = move(newPath);
		}
		newPath.insert(0, "/").insert(0, file, size);
		path.resize(parentPos);
	} while (size);
	if (type == FILE) {
		if (newPath.size()==1) // = '/'
			newPath += '.';
		else
			newPath.pop_back();
	}
	path.pop_back(); // can not be empty here
	return path = move(newPath.insert(0, path));
}

bool FileSystem::IsAbsolute(const char* path) {
	if (path[0] == '/')
		return true; // because _stat("/file") for windows or linux search on the current disk file as a absolute path
#if defined(_WIN32)
	return isalpha(path[0]) && path[1]==':' && (path[2]=='/' || path[2]=='\\'); // "C:" is a file (relative path)
#else
	return false;
#endif
}

string& FileSystem::MakeRelative(string& path) {
	UInt32 count(0);
#if defined(_WIN32)
	if (isalpha(path[0]) && path[1] == ':' && (path[2] == '/' || path[2] == '\\'))
		count += 3;
#endif
	while (path[count] == '/' || path[count] == '\\')
		++count;
	return path.erase(0, count);
}

bool FileSystem::ResolveFileWithPaths(const char* paths, string& file) {
	string path;
	String::ForEach forEach([&file,&path](UInt32 index,const char* value) {
		path.assign(value);
#if defined(_WIN32)
			// "path" => path
			if (!path.empty() && path[0] == '"' && path.back() == '"') {
				path.erase(path.front());
				path.resize(path.size()-1);
			}
#endif
		if (Exists(MakeFolder(path).append(file))) {
			file = move(path);
			return false;
		}
		return true;
	});
#if defined(_WIN32)
	return String::Split(paths, ";", forEach) == string::npos;
#else
	return String::Split(paths, ":", forEach) == string::npos;
#endif
}

bool FileSystem::IsFolder(const string& path) {
	return path.empty() || path.back()=='/' || path.back()=='\\';
}

bool FileSystem::IsFolder(const char* path) {
	size_t size(strlen(path)); 
	if (!size)
		return true;
	char c(path[size - 1]);
	return c == '/' || c == '\\';
}
	

string& FileSystem::MakeFolder(string& path) {
	// must allow a name concatenation
	if (!IsFolder(path)) {
#if defined(_WIN32)
		if (path.size() == 2 && path.back()==':' && isalpha(path.front()))
			path.insert(0, "./"); // to avoid to transform relative file "c:" in absolute file "c:/", use instead of the form "./c:/
#endif
		path += '/';
	}
	return path;
}

string& FileSystem::MakeFile(string& path) {
	if (!IsFolder(path))
		return path;

	if (path.empty())
		return path = '.';

	do {
		if (path.empty()) // was absolute
			return path.assign("/.");
		path.pop_back();
	} while (IsFolder(path));
	
#if defined(_WIN32)
	if (path.size() == 2 && path.back() == ':' && isalpha(path.front()))
		return path.append("/.");
#endif
	return path;
}


} // namespace Mona
