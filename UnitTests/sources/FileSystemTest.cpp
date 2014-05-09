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
#include "Mona/FileSystem.h"

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
	Path.assign(Home);
	Path.append(".MonaFileSystemTest");

	vector<string> directories;
	CHECK(FileSystem::Unpack(Path, directories).back() == ".MonaFileSystemTest");

	directories.emplace_back("SubFolder");
	directories.emplace_back("Folder");

	CHECK(FileSystem::Pack(directories,Path2).compare(Path.size(),Path2.size()-Path.size(),"/SubFolder/Folder")==0);
}

ADD_TEST(FileSystemTest, Creation) {
	Exception ex;
	FileSystem::CreateDirectories(ex,Path2);
	CHECK(!ex);
	CHECK(!FileSystem::Exists(Path2));
	CHECK(FileSystem::Exists(Path2, true));

	FileSystem::Remove(ex,Path,true);
	CHECK(!ex);
	CHECK(!FileSystem::Exists(Path,true));

	CHECK(FileSystem::CreateDirectory(Path));
	CHECK(FileSystem::MakeDirectory(Path).back() == '/');
	CHECK(FileSystem::Exists(Path));
	CHECK(FileSystem::MakeFile(Path).back() == 't');
	CHECK(!FileSystem::Exists(Path));

	Path2.assign(Path);
	Path2.append("s/");
	if (FileSystem::Exists(Path2))
		CHECK(FileSystem::Remove(ex,Path2) && !ex);
	CHECK(FileSystem::Rename(Path, Path2));
	CHECK(!FileSystem::Exists(Path,true));
	CHECK(FileSystem::Exists(Path2));
	CHECK(FileSystem::MakeFile(Path2).back() == 's');
}

ADD_TEST(FileSystemTest, Properties) {
	Exception ex;
	FileSystem::Attributes attributes;
	FileSystem::GetAttributes(ex, Path2,attributes);
	CHECK(!ex)
	Time lastModified;
	CHECK(FileSystem::GetLastModified(ex, Path2,lastModified)==attributes.lastModified && !ex);
	CHECK(attributes.size==0 && FileSystem::GetSize(ex, Path2)==0 && ex);
	string value;
	CHECK(FileSystem::GetName(Path2,value)==".MonaFileSystemTests");
	CHECK(FileSystem::GetBaseName(Path2,value)=="");
	CHECK(FileSystem::GetExtension(Path2,value)=="MonaFileSystemTests");
}

ADD_TEST(FileSystemTest, Deletion) {
	Exception ex;
	CHECK(FileSystem::Remove(ex,Path2) && !ex);
}

