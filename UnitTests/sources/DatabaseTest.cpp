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
#include "Mona/Database.h"
#include "Mona/FileSystem.h"

using namespace std;
using namespace Mona;


class Loader : public DatabaseLoader {
public:
	void onDataLoading(const std::string& path, const char* value, UInt32 size) {
		CHECK(strlen(path.c_str()) == 0 && size == 5 && memcmp("salut", value, size) == 0 ||
				memcmp(path.c_str(), "/Test", 5) == 0 && size == 8 && memcmp("aur\0voir", value, size) == 0);
	}
};

Database	data;
string		path;
Loader		loader;

ADD_TEST(DatabaseTest, Load) {
	// create base of test
	Exception ex;
	CHECK(FileSystem::GetHome(path));
	path.append(".MonaDatabaseTests");
	data.load(ex, path, loader);
	CHECK(!ex);
}


ADD_TEST(DatabaseTest, Add) {
	Exception ex;
	CHECK(data.add(ex, "", (const UInt8*)"salut", 5));
	CHECK(data.add(ex, "Test", (const UInt8*)"aurevoir", 8));
	CHECK(data.add(ex, "Test", (const UInt8*)"aur\0voir", 8));
	CHECK(!ex);

	data.flush();
	CHECK(FileSystem::Exists(path+"\\"));
	CHECK(FileSystem::Exists(path + "/Test/"));
}

ADD_TEST(DatabaseTest, Reload) {
	// create base of test
	Exception ex;
	data.load(ex,path,loader);
	CHECK(!ex);
}

ADD_TEST(DatabaseTest, Remove) {
	// create base of test
	Exception ex;
	CHECK(data.remove(ex, "/Test/NoExists"));
	CHECK(data.remove(ex, ""));
	CHECK(!ex);

	data.flush();
	CHECK(!FileSystem::Exists(path + "/"));
}

