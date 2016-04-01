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
class File : public virtual Object {
public:
	File() : _isNull(true), _attributesLoaded(false),_resolved(false),_type(FileSystem::FILE) {}

	template <typename ...Args>
	File(const char* value,Args... args) : _attributesLoaded(false),_isNull(true),_resolved(false),_type(FileSystem::FILE) { if(value) setPath(value,args ...); }

	template <typename ...Args>
	File(const std::string& value,Args... args) { setPath(value,args ...); }

	operator bool() const { return !_isNull; }

	// properties
	const std::string&	path() const { return _path; }
	const std::string&	name() const { return _name; }
	const std::string&	baseName() const { return _baseName; }
	const std::string&	extension() const { return _extension; }
	const std::string&  parent() const { return _parent; }
	bool				isFolder() const { return _type==FileSystem::FOLDER; }
	bool				isAbsolute() const { return _isAbsolute; }

	
	// physical disk file
	bool		exists(bool refresh = false) const;
	UInt32		size(bool refresh = false) const;
	Int64		lastModified(bool refresh = false) const;


	// setters
	bool setName(const std::string& value) { return setName(value.c_str()); }
	bool setName(const char* value);
	bool setBaseName(const std::string& value) { return setBaseName(value.c_str()); }
	bool setBaseName(const char* value);
	bool setExtension(const std::string& value) { return setExtension(value.c_str()); }
	bool setExtension(const char* value);
	File& makeFolder();
	File& makeFile();
	File& makeAbsolute();
	File& makeRelative();
	File& resolve();

	template <typename ...Args>
	File& setPath(Args&&... args) {
		_isNull = false;
		_path = std::move(String::Format(_pathPrefix, args ...)); // use _pathPrefix to allow a "File.setPath(...,File.path(),...)"
		
		std::size_t extPos;
		_type = FileSystem::GetFile(_path,_name,extPos,_parent);
		_baseName.assign(_name,0,extPos);
		if (extPos != std::string::npos)
			_extension.assign(_name, extPos+1, std::string::npos);
		else
			_extension.clear();
		_isAbsolute = FileSystem::IsAbsolute(_path);
		_resolved = _attributesLoaded = false;
		return *this;
	}

	template <typename ...Args>
	File& setParent(Args&&... args) {
		if (_isNull)
			setPath("");
		_parent = std::move(FileSystem::MakeFolder(String::Format(_pathPrefix, args ...))); // use _pathPrefix to allow a "File.setPath(...,File.path(),...)"
		_path.assign(_parent).append(_name);
		if (_type == FileSystem::FOLDER)
			_path += '/';
		_isAbsolute = FileSystem::IsAbsolute(_parent);
		_resolved = _attributesLoaded = false;
		return *this;
	}

	static const File	Home;
	static const File	CurrentApp;
	static const File	CurrentDir;
private:
	const FileSystem::Attributes& attributes(bool refresh) const;
	void computePathFromName();

	std::string			 _path;
	std::string			 _pathPrefix;
	std::string			 _name;
	std::string			 _baseName;
	std::string			 _extension;
	std::string			 _parent;
	FileSystem::Type	 _type;
	bool				 _isNull;
	bool				 _resolved;
	bool				 _isAbsolute;

	mutable std::mutex				_mutex;
	mutable FileSystem::Attributes	_attributes;
	mutable bool					_attributesLoaded;

	
};


} // namespace Mona
