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
#include "Mona/PersistentData.h"
#include "Mona/FileSystem.h"

using namespace std;
using namespace Mona;


static PoolBuffers		_PoolBuffers;
static PersistentData	_Data(_PoolBuffers);
static string			_Path;
static PersistentData::ForEach	_ForEach([](const string& path, const UInt8* value, UInt32 size) {
	CHECK((strlen(path.c_str()) == 0 && size == 5 && memcmp("salut", value, size) == 0) ||
		(path.compare("/Test") == 0 && size == 8 && memcmp("aur\0voir", value, size) == 0) ||
		(path.compare("/Sub") == 0 && size == 3 && memcmp("val", value, size) == 0));
});


ADD_TEST(PersistentData, Load) {
	// create base of test
	Exception ex;
	FileSystem::MakeFolder(_Path.assign(FileSystem::GetHome("")).append(".MonaDatabaseTests/"));
	_Data.load(ex, _Path, _ForEach);
	CHECK(!ex)
}


ADD_TEST(PersistentData, Add) {
	Exception ex;
	CHECK(_Data.add(ex, "", BIN EXPAND("salut")) && !ex);
	CHECK(_Data.add(ex, "Test", BIN EXPAND("aurevoir")) && !ex);
	CHECK(_Data.add(ex, "Test", BIN EXPAND("aur\0voir")) && !ex);
	CHECK(_Data.add(ex, "/Test/../Sub", BIN EXPAND("val")) && !ex);

	_Data.flush();
	CHECK(FileSystem::Exists(_Path));
	CHECK(FileSystem::Exists(_Path+"Test/"));
	CHECK(FileSystem::Exists(_Path+"Sub/"));
}

ADD_TEST(PersistentData, Reload) {
	// create base of test
	Exception ex;
	_Data.load(ex,_Path,_ForEach);
	CHECK(!ex);
}

ADD_TEST(PersistentData, Remove) {
	// create base of test
	Exception ex;
	CHECK(_Data.remove(ex, "/Test/NoExists") && !ex);
	CHECK(_Data.remove(ex, "/Sub") && !ex);
	CHECK(_Data.remove(ex, "") && !ex);
	_Data.flush();

	CHECK(!FileSystem::Exists(_Path + "Sub/"));

	CHECK(_Data.remove(ex, "Test") && !ex);
	CHECK(_Data.remove(ex, "") && !ex);
	_Data.flush();

	CHECK(!FileSystem::Exists(_Path));;
}

