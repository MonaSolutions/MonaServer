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
#undef CreateDirectory

namespace Mona {

class FileSystem : virtual Static {
public:
	struct Attributes {
		Attributes() : size(0), lastModified(0), isDirectory(false) {}
		Time	lastModified;
		UInt32	size;
		bool	isDirectory;
	};

	/// In giving a path with /, it tests one folder existance, otherwise file existance (test windows device without / => C:)
	static bool			Exists(const std::string& path,bool any=false);
	static bool			IsAbsolute(const std::string& path);
	
	/// Doesn't work for a folder
	static UInt32		GetSize(Exception& ex,const std::string& path);

	static Time&		GetLastModified(Exception& ex,const std::string& path, Time& time);
	static Attributes&	GetAttributes(Exception& ex, const std::string& path, Attributes& attributes);

	static std::string& GetName(const std::string& path, std::string& value);
	static std::string& GetBaseName(const std::string& path, std::string& value);
	static std::string& GetExtension(const std::string& path,std::string& value);
	static bool			GetApplicationCurrent(std::string& path);
	static bool			GetCurrent(std::string& path);
	static bool			GetHome(std::string& path);

	static void			RegisterForDeletion(const std::string& path);
	static bool			Remove(const std::string& path);
	static void			RemoveAll(Exception& ex, const std::string& path);
	static bool			Rename(const std::string& fromPath, const std::string& toPath) { return rename(fromPath.c_str(), toPath.c_str()) == 0; }
	static bool			CreateDirectory(const std::string& path);
	static void			CreateDirectories(Exception& ex, const std::string& path);
	static std::string&	MakeDirectory(std::string& path);
	static std::string&	MakeFile(std::string& path);
	
	static bool							ResolveFileWithPaths(const std::string& paths, std::string& file);
	static std::vector<std::string>&	Unpack(const std::string& path, std::vector<std::string>& values);
	static std::string&					Pack(const std::vector<std::string>& values, std::string& path);

	
private:
	static const std::string _PathSeparator;
};


} // namespace Mona
