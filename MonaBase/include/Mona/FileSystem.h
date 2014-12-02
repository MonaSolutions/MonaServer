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
#include "Mona/MapParameters.h"
#include "Mona/Time.h"
#include "Mona/Exceptions.h"
#include <vector>
#if defined(_WIN32)
#include "windows.h"
#undef CreateDirectory
#endif

namespace Mona {

class FileSystem : virtual Static {
public:
	struct Attributes {
		Attributes() : size(0), lastModified(0) {}
		Time	lastModified;
		UInt32	size;
	};

	typedef std::function<void(const std::string&)> ForEach; /// FileSystem::ListDir function type handler

	/// Iterate over files under directory path
	static UInt32	Paths(Exception& ex, const std::string& path, const ForEach& forEach) { return Paths(ex, path.c_str(), forEach); }
	static UInt32	Paths(Exception& ex, const char* path, const ForEach& forEach);

	/// In giving a path with /, it tests one folder existance, otherwise file existance (test windows device without / => C:)
	static bool			Exists(const std::string& path) { return Exists(path.c_str()); }
	static bool			Exists(const char* path);
	static bool			IsAbsolute(const std::string& path);
	static bool			IsAbsolute(const char* path);
	static bool			IsFolder(const std::string& path);
	static bool			IsFolder(const char* path);
	
	static Attributes&	GetAttributes(Exception& ex, const std::string& path, Attributes& attributes) { return GetAttributes(ex,path.data(),attributes); }
	static Attributes&	GetAttributes(Exception& ex, const char* path, Attributes& attributes);

	static std::string& GetName(const char* path, std::string& value) { return GetName(value.assign(path)); }
	static std::string& GetName(const std::string& path, std::string& value) { return GetName(value.assign(path)); }
	static std::string& GetName(std::string& path);

	static std::string& GetBaseName(const char* path, std::string& value) { return GetBaseName(value.assign(path)); }
	static std::string& GetBaseName(const std::string& path, std::string& value) { return GetBaseName(value.assign(path)); }
	static std::string& GetBaseName(std::string& path);

	static std::string& GetExtension(const char* path,std::string& value) { return GetExtension(value.assign(path)); }
	static std::string& GetExtension(const std::string& path,std::string& value) { return GetExtension(value.assign(path)); }
	static std::string& GetExtension(std::string& path);

	static UInt32		GetSize(Exception& ex,const char* path);
	static UInt32		GetSize(Exception& ex, const std::string& path) { return GetSize(ex, path); }
	static Time&		GetLastModified(Exception& ex,const std::string& path, Time& time) { return GetLastModified(ex, path.c_str(),time); }
	static Time&		GetLastModified(Exception& ex, const char* path, Time& time);
	
	static std::string&	GetParent(const char* path, std::string& value)  { return GetParent(value.assign(path)); }
	static std::string&	GetParent(std::string& path, std::string& value)  { return GetParent(value.assign(path)); }
	static std::string&	GetParent(std::string& path);

	static bool			GetCurrentApplication(std::string& path);
	static bool			GetCurrent(std::string& path);
	static bool			GetHome(std::string& path);

	static bool			Remove(Exception& ex, const std::string& path, bool all = false) { return Remove(ex, path.c_str(), all); }
	static bool			Remove(Exception& ex, const char* path, bool all = false);
	static bool			Rename(const std::string& fromPath, const std::string& toPath) { return rename(fromPath.c_str(), toPath.c_str()) == 0; }
	static bool			CreateDirectory(const std::string& path) { return CreateDirectory(path.c_str()); }
	static bool			CreateDirectory(const char* path);
	static void			CreateDirectories(Exception& ex, const std::string& path);
	static std::string&	MakeDirectory(std::string& path) { return MakeFile(path).append("/");  }
	static std::string&	MakeFile(std::string& path);
	
	static bool							ResolveFileWithPaths(const char* paths, std::string& file);
	static bool							ResolveFileWithPaths(const std::string& paths, std::string& file) { return ResolveFileWithPaths(paths.data(), file); }
	static std::vector<std::string>&	Unpack(const std::string& path, std::vector<std::string>& values) { return Unpack(path.c_str(), values); }
	static std::vector<std::string>&	Unpack(const char* path, std::vector<std::string>& values);
	static std::string&					Pack(const std::vector<std::string>& values, std::string& path);

};


} // namespace Mona
