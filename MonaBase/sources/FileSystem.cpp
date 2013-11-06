/*
Copyright 2013 Mona - mathieu.poux[a]gmail.com

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
#else
    #include "limits.h"
    #include "pwd.h"
#endif
#include "Mona/FileSystem.h"
#include <set>


namespace Mona {

using namespace std;


class TempPaths : virtual Object {
public:
	virtual ~TempPaths() {
		for (const string& path : _paths)
			FileSystem::Remove(path);
	}

	void add(const string& path) {
		lock_guard<mutex> lock(_mutex);
		_paths.insert(path); // TODO make absolute!!
	}

private:
	set<string> _paths;
	mutex       _mutex;
};

static TempPaths Temps;

// TODO eliminate everywhere the VMS support!
#if defined(_WIN32)
const string FileSystem::_PathSeparator(";");
#else
const string FileSystem::_PathSeparator(":");
#endif

void FileSystem::RegisterForDeletion(const string& path) {
	Temps.add(path);
}

Time& FileSystem::GetLastModified(Exception& ex, const string& path, Time& time) {
	struct stat status;
	status.st_mtime = 0;
	if (stat(path.c_str(), &status) != 0)
		ex.set(Exception::FILE, "File ", path, " doesn't exist");
	time.update(chrono::system_clock::from_time_t(status.st_mtime));
	return time;
}

UInt32 FileSystem::GetSize(Exception& ex,const string& path) {
	struct stat status;
	status.st_size = 0;
	if (stat(path.c_str(), &status) != 0)
		ex.set(Exception::FILE, "File ", path, " doesn't exist");
	return (UInt32)status.st_size;
}

bool FileSystem::Exists(const string& path) {
	struct stat status;
	return (stat(path.c_str(), &status) == 0);
}


bool FileSystem::CreateDirectory(const string& path) {
	if (Exists(path))
		return true;
#if defined(_WIN32)
	return CreateDirectoryA(path.c_str(), 0) != 0;
#else
    return (mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) == 0);
#endif
}



// TODO test with TestUnits
string& FileSystem::GetName(const string& path, string& value) {
	value.assign(path);
	auto separator = value.find_last_of("/\\");
	if (separator != string::npos)
		value.erase(0, separator + 1);
	return value;
}

// TODO test with TestUnits
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

// TODO test with TestUnits
bool FileSystem::IsFile(const string& path) {
	auto dot = path.find_last_of('.');
	auto separator = path.find_last_of("/\\");
	if (dot != string::npos && (separator == string::npos || dot > separator))
		return true;
	return false;
}


// TODO test with TestUnits
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

string& FileSystem::MakeDirectory(string& path) {
	auto dot = path.find_last_of('.');
	auto separator = path.find_last_of("/\\");
	if (dot != string::npos && (separator == string::npos || dot > separator)) {
		path.resize(separator + 1);
		return path;
	}

	string::size_type n = path.size();
#if defined(_WIN32)
	if (n == 0)
		path.assign("C:\\");
	else if (n > 0 && (path[n - 1] != '\\' || path[n - 1] != '/'))
		path.append("\\");
#else
	if (n > 0 && path[n - 1] != '/')
		path.append("/");
#endif
	return path;
}


// TODO test units!
string& FileSystem::Pack(const vector<string>& values, string& path) {
	path.clear();
	bool first = true;
	for (const string& value : values) {
#if defined(_WIN32)
		if (first) {
			if (value.empty() || value[value.size() - 1] != ':')
				path.append(1, '\\');
			first = false;
		} else
			path.append(1, '\\');
#else
		path.append(1, '/');
#endif
		path.append(value);
	}
	return path;
}

// TODO test units!
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
	if (!Util::Environment().getString("HOMEDRIVE", path) || !Util::Environment().getString("HOMEPATH", path)) {
		// system directory
		char buffer[MAX_PATH];
		DWORD n = GetSystemDirectoryA(buffer, sizeof(buffer));
		if (n <= 0 || n >= sizeof(buffer))
			return false;
		path.assign(buffer);
	}

	string::size_type n = path.size();
	if (n > 0 && (path[n - 1] != '\\' || path[n - 1] != '/'))
		path.append("\\");
	return true;
	
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
	string::size_type n = path.size();
	if (n > 0 && path[n - 1] != '/')
		path.append("/");

#endif
	return true;
}

// TODO test unit
bool FileSystem::IsAbsolute(const string& path) {
#if defined(_WIN32)
	return !path.empty() && isalpha(path[0]) && (path.size()<2 || path[1]==':');
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

string& FileSystem::GetCurrent(string& path) {

#if defined(_WIN32)
    int len = GetCurrentDirectoryW(0, NULL);
    if (len > 0) {
        Buffer<char *> buff(len);
        len = GetModuleFileNameA(0, buff.begin());
    }

    if (len <= 0)
        FATAL_ERROR("cannot get current directory");

    path.assign(buffer.data());
#else
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)))
        path.assign(cwd);
    else
        FATAL_ERROR("cannot get current directory");
#endif

    return path;
}

} // namespace Mona
