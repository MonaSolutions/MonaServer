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

#include "Mona/File.h"
#include "Mona/Util.h"

namespace Mona {

using namespace std;


const FileSystem::Attributes& File::attributes(bool refresh) const {
	if (!_attributesLoaded || refresh) {
		FileSystem::GetAttributes(_path, _attributes);
		_attributesLoaded = true;
	}
	return _attributes;
}


bool File::exists(bool refresh) const {
	if (_isNull)
		return false;
	lock_guard<mutex> lock(_mutex);
	return attributes(refresh); 
}

UInt32 File::size(bool refresh) const {
	if (_isNull)
		return 0;
	lock_guard<mutex> lock(_mutex);
	return attributes(refresh).size;
}

Int64 File::lastModified(bool refresh) const {
	if (_isNull)
		return 0;
	lock_guard<mutex> lock(_mutex);
	return attributes(refresh).lastModified;
}

bool File::setName(const char* value) {

	const char* name(strrpbrk(value, "/\\"));
	if (name)
		value = name+1;

	// forbid . or .. or empty name
	if (!*value || strcmp(value, ".") == 0 || strcmp(value, "..") == 0)
		return false;

	if (_isNull)
		setPath();

	size_t extPos = _name.assign(value).find_last_of('.');
	_baseName.assign(_name,0,extPos);
	if (extPos != string::npos)
		_extension.assign(_name, extPos+1, string::npos);
	else
		_extension.clear();

	computePathFromName();
	return true;
}

void File::computePathFromName() {
	if (!_isAbsolute && FileSystem::IsAbsolute(_parent)) {
		if (!_pathPrefix.empty())
			_path.assign(_pathPrefix);
		else
			_pathPrefix.assign(FileSystem::MakeFolder(_path).append("../"));
	} else
		_path.assign(_parent);
	_path.append(_name);
	if (_type == FileSystem::FOLDER)
		_path += '/';
	_attributesLoaded = false;
}


bool File::setBaseName(const char* value) {

	const char* baseName(strrpbrk(value, "/\\"));
	if (baseName)
		value = baseName+1;

	// forbid . or .. or empty name
	if ((!*value && (_extension.empty() || _extension == ".")) || (strcmp(value, ".") == 0 && _extension.empty()))
		return false;

	if (_isNull)
		setPath();

	String::Format(_name, _baseName.assign(value),'.',_extension);
	computePathFromName();
	return true;
}


bool File::setExtension(const char* value) {

	const char* extension(strrpbrk(value, "/.\\"));
	if (extension)
		value = extension+1;

	// forbid . or .. or empty name
	if ((!*value && (_baseName.empty() || _baseName == ".")) || (strcmp(value, ".") == 0 && _baseName.empty()))
		return false;

	if (_isNull)
		setPath();

	String::Format(_name, _baseName,'.',_extension.assign(value));
	computePathFromName();
	return true;
}

File& File::makeFolder() {
	if (_isNull)
		return setPath(); // give current directory
	if (_type == FileSystem::FOLDER)
		return *this;
	FileSystem::MakeFolder(_path);
	_type = FileSystem::FOLDER;
	_attributesLoaded = false;
	return *this;
}

File& File::makeFile() {
	if (_isNull)
		return setPath("."); // give current directory like file
	if (_type == FileSystem::FILE)
		return *this;
	FileSystem::MakeFile(_path);
	_type = FileSystem::FILE;
	_attributesLoaded = false;
	return *this;
}

File& File::makeAbsolute() {
	if (_isNull) {
		setPath("/");
		return *this;
	}
	if (_isAbsolute)
		return *this;
	FileSystem::MakeAbsolute(_path);
	if (FileSystem::IsAbsolute(_parent)) {
		_parent.assign("/");
		_pathPrefix.clear();
	} else
		FileSystem::MakeAbsolute(_parent);
	_isAbsolute = true;
	return *this;
}

File& File::makeRelative() {
	if (_isNull) {
		setPath("");
		return *this;
	}
	if (!_isAbsolute)
		return *this;
	FileSystem::MakeRelative(_path);
	FileSystem::MakeRelative(_parent);
	if (_parent.empty())
		FileSystem::GetParent(_path, _parent);
	_isAbsolute = false;
	return *this;
}

File& File::resolve() {
	if (!_resolved) {
		if (_isNull)
			setPath();
		else if (!_isAbsolute || !FileSystem::IsAbsolute(_parent))
			FileSystem::Resolve(_parent);
		_path.assign(_parent).append(_name);
		if (_type == FileSystem::FOLDER)
			_path += '/';
		_isAbsolute = _resolved = true;
		_pathPrefix.clear();
	}
	return *this;
}

} // namespace Mona
