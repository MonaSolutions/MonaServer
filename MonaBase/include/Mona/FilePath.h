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
#include "Mona/FileSystem.h"

namespace Mona {

class FilePath : virtual Object {
public:

	FilePath() : _attributesLoaded(false) {}

	template <typename ...Args>
	FilePath(const std::string& value,Args&&... args) : _attributesLoaded(false) { set(value,args ...); }

	FilePath& operator=(const FilePath& other);

	const std::string& path() const { return _path; }
	const std::string& fullPath() const;

	const std::string&	directory() const { return _directory; }
	const std::string&	name() const { return _name.empty() ? FileSystem::GetName(_path,_name) : _name; }
	const std::string&	baseName() const { return _baseName.empty() ? FileSystem::GetBaseName(_path,_baseName) : _baseName; }
	const std::string&	extension() const { return _extension.empty() ? FileSystem::GetExtension(_path,_extension) : _extension; }

	UInt32				size() const { return attributes().size; }
	const Time&			lastModified() const { return attributes().lastModified; }
	bool				isDirectory() const { return attributes().isDirectory; }

	template <typename ...Args>
	const std::string& set(const std::string& value,Args&&... args) {
		_name.clear();
		_baseName.clear();
		_extension.clear();
		_attributesLoaded = false;
		String::Format(_buffer,value,args ...);
		_path.assign(_buffer);
		return _path;
	}
	
	template <typename ...Args>
	const std::string& directory(const std::string& value,Args&&... args) {
		_attributesLoaded = false;
		String::Format(_directory, value,args ...);
		return _directory;
	}
	
private:
	const FileSystem::Attributes& attributes() const;

	std::string		_path;
	std::string		_directory;

	mutable std::string				_fullPath;
	mutable std::string				_name;
	mutable std::string				_baseName;
	mutable std::string				_extension;
	mutable FileSystem::Attributes	_attributes;
	mutable bool					_attributesLoaded;

	std::string		_buffer;
};


} // namespace Mona
