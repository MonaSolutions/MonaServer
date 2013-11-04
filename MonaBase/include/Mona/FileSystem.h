/*
Copyright 2013 Mona - mathieu.poux[a]gmail.com

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
#undef CreateDirectory

namespace Mona {


class FileSystem : virtual Static {
public:

	static bool			Exists(const std::string& path);
	static bool			IsFile(const std::string& path);
	static bool			IsDirectory(const std::string& path) { return !IsFile(path); }
	static bool			IsAbsolute(const std::string& path);
	
	static UInt32		GetSize(Exception& ex,const std::string& path);
	static Time&		GetLastModified(Exception& ex,const std::string& path, Time& time);

	static std::string& GetName(const std::string& path, std::string& value);
	static std::string& GetBaseName(const std::string& path, std::string& value);
	static std::string& GetExtension(const std::string& path,std::string& value);
	
	static bool			GetHome(std::string& path);

	static void			RegisterForDeletion(const std::string& path);
	static bool			Remove(const std::string& path) { return remove(path.c_str()) == 0; }
	static bool			Rename(const std::string& fromPath, const std::string& toPath) { return rename(fromPath.c_str(), toPath.c_str()) == 0; }
	static bool			CreateDirectory(const std::string& path);
	static std::string&	MakeDirectory(std::string& path);
	
	static bool							ResolveFileWithPaths(const std::string& paths, std::string& file);
	static std::vector<std::string>&	Unpack(const std::string& path, std::vector<std::string>& values);
	static std::string&					Pack(const std::vector<std::string>& values, std::string& path);
	
private:
	static const std::string _PathSeparator;
};


} // namespace Mona
