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

/// \class Record access to name, extension and basename for a faster response
///
class Path : public virtual Object {
public:
	Path() : _attributesLoaded(false) {}

	template <typename ...Args>
	Path(const std::string& value,Args... args) : _attributesLoaded(false) { set(value,args ...); }

	template <typename ...Args>
	Path(const char* value,Args... args) : _attributesLoaded(false) { set(value,args ...); }

	Path& operator=(const Path& other);

	const std::string&	toString() const { return _path; }
	const std::string&	operator()() const { return _path; }

	const std::string&	name() const { return _name.empty() ? FileSystem::GetName(_path,_name) : _name; }
	const std::string&	baseName() const { return _baseName.empty() ? FileSystem::GetBaseName(_path,_baseName) : _baseName; }
	const std::string&	extension() const { return _extension.empty() ? FileSystem::GetExtension(_path,_extension) : _extension; }

	UInt32				size() const { return attributes().size; }
	const Time&			lastModified() const { return attributes().lastModified; }
	const std::string&  parent() const { return _parent.empty() ? FileSystem::GetParent(_parent.assign(_path)) : _parent; }
	bool				isDirectory() const { return attributes().isDirectory; }
	void				update() const { _attributesLoaded = false; }
	
	template <typename ...Args>
	void append(Args&&... args) {
		_name.clear();
		_baseName.clear();
		_extension.clear();
		_parent.clear();
		_attributesLoaded = false;
		String::Append(_path, args ...);
	}

	template <typename ...Args>
	void set(Args&&... args) {
		_path.clear();
		append(args ...);
	}
	
private:
	const FileSystem::Attributes& attributes() const;

	std::string		_path;

	mutable std::string				_name;
	mutable std::string				_baseName;
	mutable std::string				_extension;
	mutable std::string				_parent;
	mutable FileSystem::Attributes	_attributes;
	mutable bool					_attributesLoaded;
};


} // namespace Mona
