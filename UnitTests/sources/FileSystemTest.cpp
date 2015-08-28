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

#include "Test.h"
#if defined(_WIN32)
	#include "windows.h"
#endif
#include "Mona/FileSystem.h"
#include <fstream>

using namespace std;
using namespace Mona;

static string Home;
static string Path;
static string Path2;

ADD_TEST(FileSystemTest, Static) {
	CHECK(FileSystem::GetCurrent(Home));
	CHECK(!Home.empty() && FileSystem::IsAbsolute(Home));
	CHECK(FileSystem::GetHome(Home));
	CHECK(!Home.empty() && FileSystem::IsAbsolute(Home));
}

ADD_TEST(FileSystemTest, Resolution) {
	Path.assign("/path/sub/..");
	CHECK(FileSystem::GetParent(Path) == "/");
	Path.assign("/..");
	CHECK(FileSystem::GetParent(Path) == ".");


	Path.assign(Home);
	Path.append(".MonaFileSystemTest");

	vector<string> directories;
	CHECK(FileSystem::Unpack(Path, directories).back() == ".MonaFileSystemTest");

	directories.emplace_back("SubFolder");
	directories.emplace_back("Folder/");

	CHECK(FileSystem::Pack(directories,Path2).compare(Path.size(),Path2.size()-Path.size(),"/SubFolder/Folder/")==0);
}

ADD_TEST(FileSystemTest, Creation) {
	Exception ex;
	FileSystem::CreateDirectories(ex,Path2);
	CHECK(!ex);
	CHECK(FileSystem::Exists(Path2));

	CHECK(FileSystem::Remove(ex,Path,true) && !ex);
	CHECK(!FileSystem::Exists(Path));

	CHECK(FileSystem::CreateDirectory(Path));
	CHECK(FileSystem::MakeDirectory(Path).back() == '/');
	CHECK(FileSystem::Exists(Path));
	CHECK(FileSystem::MakeFile(Path).back() == 't');
	CHECK(!FileSystem::Exists(Path));

	Path2.assign(Path);
	Path2.append("s/");
	if (FileSystem::Exists(Path2))
		CHECK(FileSystem::Remove(ex,Path2,true) && !ex);
	CHECK(FileSystem::Rename(Path, Path2));
	CHECK(!FileSystem::Exists(Path));
	CHECK(FileSystem::Exists(Path2));
	CHECK(FileSystem::MakeFile(Path2).back() == 's');
}

ADD_TEST(FileSystemTest, Properties) {
	Exception ex;
	FileSystem::Attributes attributes;
	FileSystem::GetAttributes(ex, Path2,attributes);
	CHECK(ex)
	string value;
	CHECK(FileSystem::GetName(Path2,value)==".MonaFileSystemTests");
	CHECK(FileSystem::GetBaseName(Path2,value)=="");
	CHECK(FileSystem::GetExtension(Path2,value)=="MonaFileSystemTests");
	ex.set(Exception::NIL);
	FileSystem::GetAttributes(ex, Path2.append("/"),attributes);
	CHECK(!ex)
}

ADD_TEST(FileSystemTest, Utf8) {
	Exception ex;
#if defined(_WIN32)
	String::Format(Path, Home, L"Приве́т नमस्ते שָׁלוֹם");
#else
	String::Format(Path, Home, "Приве́т नमस्ते שָׁלוֹם");
#endif
	
	// file
#if defined(_WIN32)
	wchar_t wFile[_MAX_PATH];
	MultiByteToWideChar(CP_UTF8, 0, Path.c_str(), -1, wFile, _MAX_PATH);
	ofstream(wFile).close();
#else
	ofstream(Path.c_str()).close();
#endif
	CHECK(FileSystem::Exists(Path))
	CHECK(FileSystem::Remove(ex, Path) && !ex);
	CHECK(!FileSystem::Exists(Path))

	// folder
	Path.append("/");
	if (FileSystem::Exists(Path))
		CHECK(FileSystem::Remove(ex, Path) && !ex);
	CHECK(FileSystem::CreateDirectory(Path));
	CHECK(FileSystem::Exists(Path))
	CHECK(FileSystem::Remove(ex, Path) && !ex);
	CHECK(!FileSystem::Exists(Path))
}

ADD_TEST(FileSystemTest, Deletion) {
	Exception ex;
	CHECK(FileSystem::Remove(ex,Path2,true) && !ex);
}

