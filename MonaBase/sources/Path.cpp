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

#include "Mona/Path.h"

namespace Mona {

using namespace std;

Path& Path::operator=(const Path& other) {
	if ((_attributesLoaded = other._attributesLoaded)) {
		_attributes = other._attributes;
		_exists = other._exists;
	}
	_folder = other._folder;
	_extension = other._extension;
	_path = other._path;
	_name = other._name;
	_baseName = other._baseName;
	_parent = other._parent;
	return *this;
}

const FileSystem::Attributes& Path::attributes() const {
	if (_attributesLoaded)
		return _attributes;
	Exception ex;
	FileSystem::GetAttributes(ex, _path, _attributes);
	_attributesLoaded = true;
	_exists = !ex;
	return _attributes;
}



} // namespace Mona
