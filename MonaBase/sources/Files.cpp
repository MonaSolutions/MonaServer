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


#include "Mona/Files.h"
#include "Mona/FileSystem.h"

namespace Mona {

using namespace std;

Files::Files(Exception& ex, const string& path) : _failed(false), _directory(path) {
	_failed = false;
	int err = 0;
	FileSystem::MakeDirectory(_directory);
#if defined(_WIN32)
	string directory(_directory);
	directory.append("*");
	_fileHandle = FindFirstFile(directory.c_str(), &_fileData);
	if (_fileHandle == INVALID_HANDLE_VALUE) {
		if ((err = GetLastError()) == ERROR_NO_MORE_FILES)
			return;
	} else {
		if (strlen(_fileData.cFileName) == 1 && memcmp(_fileData.cFileName, ".", 1)==0 || strlen(_fileData.cFileName) == 2 && memcmp(_fileData.cFileName, "..", 2)==0)
			next();
		else
			_files.emplace_back(_fileData.cFileName);
		return;
	}
#else
	_pDirectory = opendir(_directory.c_str());
	if (_pDirectory) {
		next();
		return;
	}
	err = errno;
#endif
	_failed = true;
	string error;
	STRERROR(err, error); // TODO test
	ex.set(Exception::FILE, error);
}

Files::~Files() {
#if defined(_WIN32)
	if (_fileHandle != INVALID_HANDLE_VALUE)
		FindClose(_fileHandle);
#else
	if (_pDirectory)
		closedir(_pDirectory);
#endif
}


bool Files::next() {
	if (_failed)
		return false;
#if defined(_WIN32)
	while (FindNextFile(_fileHandle, &_fileData) != 0) {
		if (strlen(_fileData.cFileName) == 1 && memcmp(_fileData.cFileName, ".", 1) == 0 || strlen(_fileData.cFileName) == 2 && memcmp(_fileData.cFileName, "..", 2) == 0)
			continue;
		_files.emplace_back(_fileData.cFileName);
		return true;
	}
#else
	struct dirent* pEntry(NULL);
	while(pEntry = readdir(_pDirectory)) {
		if (strlen(pEntry->d_name) == 1 && memcmp(pEntry->d_name, ".", 1)==0 || strlen(pEntry->d_name) == 2 && memcmp(pEntry->d_name, "..", 2)==0)
			continue;
		_files.emplace_back(pEntry->d_name);
		return true;
	}
#endif
	_failed = true;
	return false;
}

DirectoryIterator::DirectoryIterator(Files& files, bool end) : _it(end ? _files._files.end() : _files._files.begin()), _files(files) {
}

DirectoryIterator DirectoryIterator::operator++() {
	if (_files.next())
		++_it;
	else
		_it = _files._files.end();
	return *this;
}

DirectoryIterator DirectoryIterator::operator--() {
	if (_it != _files._files.begin())
		--_it;
	return *this;
}

const string& DirectoryIterator::operator*() {
	return _it == _files._files.end() ? String::Empty : *_it;
}


} // namespace Mona
