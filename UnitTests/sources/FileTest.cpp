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
#include "Mona/File.h"
#include <fstream>

using namespace std;
using namespace Mona;

ADD_TEST(FileTest, Getters) {

	File file;
	CHECK(!file);
	file.setPath("");
	CHECK(file);
	CHECK(file.path().empty());
	CHECK(!file.isAbsolute());
	CHECK(file.name() == File::CurrentDir.name());
	CHECK(file.baseName() == File::CurrentDir.baseName());
	CHECK(file.extension() == File::CurrentDir.extension());
	CHECK(file.parent() == File::CurrentDir.parent());
	CHECK(file.isFolder() == File::CurrentDir.isFolder());
	
}


ADD_TEST(FileTest, Setters) {

	File file;
	CHECK(file.setParent("home", "/") && file.name() == File::CurrentDir.name());
	CHECK(file.setName("test.txt") && file.path()=="home/test.txt/");
	CHECK(file.setExtension("t.") && file.name()=="test.");
	CHECK(!file.setBaseName("") && file.name()=="test.");
	CHECK(!file.makeFile().isFolder() && file.path() == "home/test.");
	CHECK(file.makeFolder().isFolder() && file.path() == "home/test./");

	CHECK(file.setPath(file.parent()).makeAbsolute().isAbsolute() && file.parent()=="/" && file.path()=="/home/");
	CHECK(!file.makeRelative().isAbsolute() && file.parent()==File::CurrentDir.path() && file.path()=="home/");
	CHECK(file.setPath("./../////").resolve().path() == File::CurrentDir.parent());
	
}

ADD_TEST(FileTest, Attributes) {
	
	CHECK(File::CurrentApp.exists());
	File file("test.txt");
	{
		ofstream ostr("test.txt");
		ostr.put('t');
		ostr.close();
	}
	CHECK(file.exists());
	CHECK(file.lastModified());
	CHECK(file.size()==1);
	Exception ex;
	CHECK(FileSystem::Delete(ex, file.path()) && !ex);
	CHECK(file.exists());
	CHECK(file.lastModified());
	CHECK(!file.exists(true));
	CHECK(!file.lastModified());

}
