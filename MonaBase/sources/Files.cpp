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
#if defined(_WIN32)
#include <windows.h>
#else
#include "dirent.h"
#endif

namespace Mona {

using namespace std;

Files::Files(Exception& ex, const string& path) : _directory(path) {
	int err = 0;
	FileSystem::MakeDirectory(_directory);
#if defined(_WIN32)
	string directory(_directory);
	directory.append("*");

	wchar_t wDirectory[_MAX_PATH];
	MultiByteToWideChar(CP_UTF8, 0, directory.c_str(), -1, wDirectory, _MAX_PATH);
	
	WIN32_FIND_DATAW fileData;
	HANDLE	fileHandle = FindFirstFileW(wDirectory, &fileData);
	if (fileHandle == INVALID_HANDLE_VALUE) {
		if ((err = GetLastError()) != ERROR_NO_MORE_FILES) {
			ex.set(Exception::FILE, "Impossible to open directory ",_directory);
			return;
		}
		return;
	}
	do {
		if (wcscmp(fileData.cFileName, L".") != 0 && wcscmp(fileData.cFileName, L"..") != 0) {
			_files.emplace_back(_directory);
			char file[_MAX_FNAME];
			WideCharToMultiByte(CP_UTF8, 0, fileData.cFileName, -1, file, _MAX_FNAME, NULL, NULL);
			_files.back().append(file);
		}
	} while (FindNextFileW(fileHandle, &fileData) != 0);
	FindClose(fileHandle);
#else
	DIR* pDirectory = opendir(_directory.c_str());
	if (!pDirectory) {
		ex.set(Exception::FILE, "Impossible to open directory ",_directory);
		return;
	}
	struct dirent* pEntry(NULL);
	while(pEntry = readdir(pDirectory)) {
		if (strcmp(pEntry->d_name, ".")!=0 && strcmp(pEntry->d_name, "..")!=0) {
			_files.emplace_back(_directory);
			_files.back().append(pEntry->d_name);
		}
	}
	closedir(pDirectory);
#endif
}



} // namespace Mona
