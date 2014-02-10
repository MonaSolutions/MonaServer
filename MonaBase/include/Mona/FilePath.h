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

/// \class Divide paths into 2 parts to make it more easy to change one of them
/// and record access to name, extension and basename for a faster response
///
/// directory : The root path (should be a directory)
/// path : The relative path (may be a directory or a file)
class FilePath : virtual Object {
public:
	FilePath() : _attributesLoaded(false) {}

	template <typename ...Args>
	FilePath(const std::string& value,Args&&... args) : _attributesLoaded(false) { set(value,args ...); }

	template <typename ...Args>
	FilePath(const char* value,Args&&... args) : _attributesLoaded(false) { set(value,args ...); }

	FilePath& operator=(const FilePath& other);

	const std::string&	path() const { return _path; }
	const std::string&	fullPath() const;

	const std::string&	directory() const { return _directory; }
	const std::string&	name() const { return _name.empty() ? FileSystem::GetName(_path,_name) : _name; }
	const std::string&	baseName() const { return _baseName.empty() ? FileSystem::GetBaseName(_path,_baseName) : _baseName; }
	const std::string&	extension() const { return _extension.empty() ? FileSystem::GetExtension(_path,_extension) : _extension; }

	UInt32				size() const { return attributes().size; }
	const Time&			lastModified() const { return attributes().lastModified; }
	bool				isDirectory() const { return attributes().isDirectory; }
	void				update() const { _attributesLoaded = false; }

	template <typename ...Args>
	void set(Args&&... args) {
		_name.clear();
		_baseName.clear();
		_extension.clear();
		_attributesLoaded = false;
		_directory.clear();
		_path.clear();
		append(args ...);
	}

	template <typename ...Args>
	const std::string& setPath(Args&&... args) {
		_path.clear();
		return appendPath(args ...);
	}

	template <typename ...Args>
	const std::string& appendPath(Args&&... args) {
		_name.clear();
		_baseName.clear();
		_extension.clear();
		_attributesLoaded = false;
		return String::Append(_path,args ...);
	}

	template <typename ...Args>
	const std::string& setDirectory(Args&&... args) {
		_directory.clear();
		return appendDirectory(args ...);
	}

	template <typename ...Args>
	const std::string& appendDirectory(Args&&... args) {
		_attributesLoaded = false;
		return String::Append(_directory,args ...);
	}
	
private:
	template <typename ...Args>
	void append(const std::string& value,Args&&... args) {
		_directory.append(value);
		append(args ...);
	}
	template <typename ...Args>
	void append(const char* value,Args&&... args) {
		_directory.append(value);
		append(args ...);
	}
	void append(const std::string& value) {
		if (value.empty())
			return;

		if (value.back() != '/' && value.back() != '\\')
			_path.assign(value);
		else
			_directory.append(value);
		if(!_directory.empty())
			FileSystem::MakeDirectory(_directory);
	}
	void append(const char* value) {
		char back = value[strlen(value) - 1];
		if (back != '/' && back != '\\')
			_path.assign(value);
		else
			_directory.append(value);
		if(!_directory.empty())
			FileSystem::MakeDirectory(_directory);
	}

	const FileSystem::Attributes& attributes() const;

	std::string		_path;
	std::string		_directory;

	mutable std::string				_fullPath;
	mutable std::string				_name;
	mutable std::string				_baseName;
	mutable std::string				_extension;
	mutable FileSystem::Attributes	_attributes;
	mutable bool					_attributesLoaded;
};


} // namespace Mona
