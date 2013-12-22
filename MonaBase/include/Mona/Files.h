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


#pragma once

#include "Mona/Mona.h"
#include "Mona/Exceptions.h"
#if defined(_WIN32)
#include "windows.h"
#else
#include "dirent.h"
#endif
#include <deque>

namespace Mona {


class Files : virtual Object {
public:
	Files(Exception& ex, const std::string& path);
	virtual ~Files();

	typedef std::deque<std::string>::const_iterator Iterator;

	const std::string& directory() const { return _directory; }

	Iterator begin() const { return _files.begin(); }
	Iterator end() const { return _files.end(); }

	UInt32 count() const { return _files.size(); }
private:

	bool next();

	std::deque<std::string>	_files;
	std::string				_directory;

#if defined(_WIN32)
	HANDLE				_fileHandle;
	WIN32_FIND_DATA		_fileData;
#else
	DIR *	_pDirectory;
#endif
	
};

} // namespace Mona
