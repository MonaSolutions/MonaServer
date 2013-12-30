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

#include "Mona/FilePath.h"

namespace Mona {

using namespace std;

FilePath& FilePath::operator=(const FilePath& other) {
	_attributes = other._attributes; 
	_attributesLoaded = other._attributesLoaded;
	_extension = other._extension;
	_fullPath = other._fullPath;
	_directory = other._directory;
	_path = other._path;
	_name = other._name;
	return *this;
}

const string& FilePath::fullPath() const {
	if (_fullPath.empty()) {
		_fullPath.assign(_directory);
		_fullPath.append(_path);
	}
	return _fullPath;
}


const FileSystem::Attributes& FilePath::attributes() const {
	if (_attributesLoaded)
		return _attributes;
	Exception ex;
	FileSystem::GetAttributes(ex, fullPath(), _attributes);
	_attributesLoaded = !ex;
	return _attributes;
}



} // namespace Mona
