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
#include "Mona/Path.h"

using namespace std;
using namespace Mona;

ADD_TEST(PathTest, CommonWin32) {

	Path file1("C:\\Users\\Public\\file.test");
	CHECK(file1()=="C:\\Users\\Public\\file.test");
	CHECK(file1.baseName()=="file");
	CHECK(file1.extension()=="test");
	CHECK(!file1.isDirectory());
	CHECK(file1.name()=="file.test");

	Path file2("C:\\Users\\Public", "/file.test");
	CHECK(file2()=="C:\\Users\\Public/file.test");
	CHECK(file2.baseName()=="file");
	CHECK(file2.extension()=="test");
	CHECK(!file2.isDirectory());
	CHECK(file2.name()=="file.test");

	Path file3("C:/Users/Public/file.test");
	CHECK(file3()=="C:/Users/Public/file.test");
	CHECK(file3.baseName()=="file");
	CHECK(file3.extension()=="test");
	CHECK(!file3.isDirectory());
	CHECK(file3.name()=="file.test");

	Path file4("C:/Users/Public/", "file.test");
	CHECK(file4()=="C:/Users/Public/file.test");
	CHECK(file4.baseName()=="file");
	CHECK(file4.extension()=="test");
	CHECK(!file4.isDirectory());
	CHECK(file4.name()=="file.test");

	Path directory1("C:/Users/Public");
	CHECK(directory1()=="C:/Users/Public");
	CHECK(directory1.baseName()=="Public");
	CHECK(directory1.extension()=="");
	CHECK(directory1.name()=="Public");

	Path directory2("C:/Users/Public/");
	CHECK(directory2()=="C:/Users/Public/");
	CHECK(directory2.baseName()=="");
	CHECK(directory2.extension()=="");
	CHECK(directory2.name()=="");
}

ADD_TEST(PathTest, CommonLinux) {

	Path file1("/user/dev/file.test");
	CHECK(file1()=="/user/dev/file.test");
	CHECK(file1.baseName()=="file");
	CHECK(file1.extension()=="test");
	CHECK(!file1.isDirectory());
	CHECK(file1.name()=="file.test");

	Path file2("/user/dev", "/file.test");
	CHECK(file2()=="/user/dev/file.test");
	CHECK(file2.baseName()=="file");
	CHECK(file2.extension()=="test");
	CHECK(!file2.isDirectory());
	CHECK(file2.name()=="file.test");
}

ADD_TEST(PathTest, Operations) {

	Path file("/user/dev/file.test");
	CHECK(file()=="/user/dev/file.test");

	file.set("/user/dev/file2.test");
	CHECK(file()=="/user/dev/file2.test");

	file.set("/user/dev/");
	CHECK(file()=="/user/dev/");
	file.append("file.test");
	CHECK(file()=="/user/dev/file.test");
	CHECK(file.baseName()=="file");
	CHECK(file.extension()=="test");
	CHECK(file.name()=="file.test");

	file.set("/user", "/dev");
	CHECK(file()=="/user/dev");
	CHECK(file.baseName()=="dev");
	CHECK(file.extension()=="");
	CHECK(file.name()=="dev");

	Path file2("/tmp", "/tmpFile.txt");
	CHECK(file2()=="/tmp/tmpFile.txt");
	CHECK(file2.baseName()=="tmpFile");
	CHECK(file2.extension()=="txt");
	CHECK(file2.name()=="tmpFile.txt");
	CHECK(!file2.isDirectory());

	file = file2;
	CHECK(file()=="/tmp/tmpFile.txt");
	CHECK(file.baseName()=="tmpFile");
	CHECK(file.extension()=="txt");
	CHECK(file.name()=="tmpFile.txt");
}
