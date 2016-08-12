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


FileSystem

Developer notes:
- FileSystem must support just local path, no remoting path (//server/...), mainly because the operation could be blocking
- FileSystem must support ".." and "." but not "~" because on windows a file can be named "~" and because "stat" on linux and on windows doesn't resolve "~"
- A folder path ends with "/" and not a file
- FileSystem must support and resolve the multiple slash and anti-slahs

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
	enum Type {
		FILE,
		FOLDER,
	};

	enum Mode {
		LOW=0,
		HEAVY
	};

	class Attributes : public virtual NullableObject {
	public:
		Attributes() : size(0), lastModified(0) {}
		Time	lastModified;
		UInt32	size;
		operator bool() const { return lastModified ? true : false; }
		Attributes& reset() { lastModified = 0; size = 0; return *this; }
	};


	typedef std::function<void(const std::string&, UInt16 level)> ForEach; /// FileSystem::ListDir function type handler

	/// Iterate over files under directory path
	static UInt32	ListFiles(Exception& ex, const std::string& path, const ForEach& forEach, Mode mode=LOW) { return ListFiles(ex, path.c_str(), forEach, mode); }
	static UInt32	ListFiles(Exception& ex, const char* path, const ForEach& forEach, Mode mode=LOW);

	/// In giving a path with /, it tests one folder existance, otherwise file existance
	static bool			Exists(const std::string& path)  { return Exists(path.data(), path.size()); }
	static bool			Exists(const char* path) { return Exists(path, strlen(path)); }
	static bool			IsAbsolute(const std::string& path) { return IsAbsolute(path.c_str()); }
	static bool			IsAbsolute(const char* path);
	static bool			IsFolder(const std::string& path);
	static bool			IsFolder(const char* path);
	
	static std::string&	MakeFolder(std::string& path);
	static std::string&	MakeFile(std::string& path);
	static std::string& MakeAbsolute(std::string& path) { if (!IsAbsolute(path)) path.insert(0,"/"); return path; }
	static std::string& MakeRelative(std::string& path);
	static std::string& Resolve(std::string& path);


	/// extPos = position of ".ext"
	static Type		GetFile(const char* path, std::string& name) { std::size_t extPos; return GetFile(path,name,extPos); }
	static Type		GetFile(const char* path, std::string& name, std::size_t& extPos) { return GetFile(path,name,extPos,(std::string&)String::Empty); }
	static Type		GetFile(const char* path, std::string& name, std::string& parent) { std::size_t extPos; return GetFile(path,name,extPos,parent); }
	static Type		GetFile(const char* path, std::string& name, std::size_t& extPos, std::string& parent) { return GetFile(path, strlen(path), name, extPos, parent); }
	
	static Type		GetFile(const std::string& path, std::string& name) { std::size_t extPos; return GetFile(path,name,extPos); }
	static Type		GetFile(const std::string& path, std::string& name, std::size_t& extPos) { return GetFile(path,name,extPos,(std::string&)String::Empty); }
	static Type		GetFile(const std::string& path, std::string& name, std::string& parent) { std::size_t extPos;  return GetFile(path,name, extPos, parent); }
	static Type		GetFile(const std::string& path, std::string& name, std::size_t& extPos, std::string& parent) { return GetFile(path.data(), path.size() , name, extPos, parent); }
	

	static std::string& GetName(std::string& value) { return GetName(value,value); }
	static std::string& GetName(const char* path, std::string& value);
	static std::string& GetName(const std::string& path, std::string& value) { return GetName(path.c_str(),value); }
	static std::string& GetBaseName(std::string& value) { return GetBaseName(value,value); }
	static std::string& GetBaseName(const char* path, std::string& value);
	static std::string& GetBaseName(const std::string& path, std::string& value) { return GetBaseName(path.c_str(),value); }
	static std::string& GetExtension(std::string& value) { return GetExtension(value,value); }
	static std::string& GetExtension(const char* path, std::string& value);
	static std::string& GetExtension(const std::string& path, std::string& value) { return GetExtension(path.c_str(),value); }
	static std::string& GetParent(std::string& value) { return GetParent(value,value); }
	static std::string& GetParent(const char* path, std::string& value)  { return GetParent(path,strlen(path),value); }
	static std::string& GetParent(const std::string& path, std::string& value)  { return GetParent(path.data(),path.size(),value); }

	static Attributes&	GetAttributes(const std::string& path, Attributes& attributes) { return GetAttributes(path.data(),path.size(),attributes); }
	static Attributes&	GetAttributes(const char* path, Attributes& attributes) { return GetAttributes(path, strlen(path),attributes); }

	static UInt32		GetSize(Exception& ex,const char* path, UInt32 defaultValue=0) { return GetSize(ex, path, strlen(path), defaultValue); }
	static UInt32		GetSize(Exception& ex, const std::string& path, UInt32 defaultValue=0) { return GetSize(ex, path.data(), path.size(), defaultValue); }
	static Time&		GetLastModified(Exception& ex,const std::string& path, Time& time) { return GetLastModified(ex, path.data(),path.size(), time); }
	static Time&		GetLastModified(Exception& ex, const char* path, Time& time) { return GetLastModified(ex, path,strlen(path), time); }

	static const char*	GetHome(const char* defaultPath=NULL) { return _Home ? _Home.c_str() : defaultPath; }
	static bool			GetHome(std::string& path) { return _Home ? (path.assign(_Home),true) : false; }
	static const char*	GetCurrentApp(const char* defaultPath=NULL) { return _CurrentApp ? _CurrentApp.c_str() : defaultPath; }
	static bool			GetCurrentApp(std::string& path) { return _CurrentApp ? (path.assign(_CurrentApp), true) : false; }
	static const char*	GetCurrentDir(const char* defaultPath=NULL) { return _CurrentDirs ? _CurrentDirs.back().c_str() : defaultPath; }
	static bool			GetCurrentDir(std::string& path) { return _CurrentDirs ? (path.assign(_CurrentDirs.back()),true) : false; }
	
	static bool			CreateDirectory(Exception& ex, const std::string& path, Mode mode = LOW) { return CreateDirectory(ex, path.data(),path.size(),mode); }
	static bool			CreateDirectory(Exception& ex, const char* path, Mode mode = LOW) { return CreateDirectory(ex, path,strlen(path),mode); }
	static bool			Rename(const std::string& fromPath, const std::string& toPath) { return rename(fromPath.c_str(), toPath.c_str()) == 0; }
	static bool			Rename(const std::string& fromPath, const char* toPath) { return rename(fromPath.c_str(), toPath) == 0; }
	static bool			Rename(const char* fromPath, const std::string& toPath) { return rename(fromPath, toPath.c_str()) == 0; }
	static bool			Rename(const char* fromPath, const char* toPath) { return rename(fromPath, toPath) == 0; }
	static bool			Delete(Exception& ex, const std::string& path, Mode mode = LOW) { return Delete(ex, path.data(), path.size(), mode); }
	static bool			Delete(Exception& ex, const char* path, Mode mode = LOW) { return Delete(ex, path, strlen(path), mode); }
	
	static bool			ResolveFileWithPaths(const char* paths, std::string& file);
	static bool			ResolveFileWithPaths(const std::string& paths, std::string& file) { return ResolveFileWithPaths(paths.data(), file); }

private:
	class Home : public std::string, public virtual NullableObject {
	public:
		Home();
		operator bool() const { return !empty(); }
	};

	class Directory : public std::string, public virtual Object {
	public:
		Directory(const std::string& value) : std::string(value) {}
		Directory(const char* value) : std::string(value) {}
		Directory(Directory&&		 other) : std::string(std::move(other)), name(std::move(other.name)), extPos(other.extPos) {}
		std::string name;
		size_t      extPos;
	};

	class CurrentDirs : public std::vector<Directory>, public virtual NullableObject {
	public:
		CurrentDirs();
		operator bool() const { return !_isNull; }
	private:
		bool _isNull;
	};

	class CurrentApp : public std::string, public virtual NullableObject {
	public:
		CurrentApp();
		operator bool() const { return !empty(); }
	};

	static Home			 _Home;
	static CurrentApp 	_CurrentApp;
	static CurrentDirs	_CurrentDirs;

	static Attributes& GetAttributes(const char* path, std::size_t size, Attributes& attributes);
	static bool Exists(const char* path, std::size_t size);
	static bool CreateDirectory(Exception& ex, const char* path, std::size_t size, Mode mode);
	static bool Delete(Exception& ex, const char* path, std::size_t size, Mode mode);
	static Time& GetLastModified(Exception& ex, const char* path, std::size_t size, Time& time);
	static UInt32 GetSize(Exception& ex, const char* path, std::size_t size, UInt32 defaultValue);
	static std::string& GetParent(const char* path, std::size_t size, std::string& value);
	static const char*  GetFile(const char* path, std::size_t& size, std::size_t& extPos, Type& type, Int32& parentPos);
	static Type			GetFile(const char* path, std::size_t size, std::string& name, std::size_t& extPos, std::string& parent);
};


} // namespace Mona
