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
#include "Mona/FilePath.h"

using namespace std;
using namespace Mona;

ADD_TEST(FilePathTest, CommonWin32) {

	FilePath file1("C:\\Users\\Public\\file.test");
	CHECK(file1.fullPath()=="C:\\Users\\Public\\file.test");
	CHECK(file1.baseName()=="file");
	CHECK(file1.directory()=="");
	CHECK(file1.extension()=="test");
	CHECK(!file1.isDirectory());
	CHECK(file1.name()=="file.test");
	CHECK(file1.path()=="C:\\Users\\Public\\file.test");

	FilePath file2("C:\\Users\\Public", "file.test");
	CHECK(file2.fullPath()=="C:\\Users\\Public/file.test");
	CHECK(file2.baseName()=="file");
	CHECK(file2.directory()=="C:\\Users\\Public/");
	CHECK(file2.extension()=="test");
	CHECK(!file2.isDirectory());
	CHECK(file2.name()=="file.test");
	CHECK(file2.path()=="file.test");

	FilePath file3("C:/Users/Public/file.test");
	CHECK(file3.fullPath()=="C:/Users/Public/file.test");
	CHECK(file3.baseName()=="file");
	CHECK(file3.directory()=="");
	CHECK(file3.extension()=="test");
	CHECK(!file3.isDirectory());
	CHECK(file3.name()=="file.test");
	CHECK(file3.path()=="C:/Users/Public/file.test");

	FilePath file4("C:/Users/Public", "file.test");
	CHECK(file4.fullPath()=="C:/Users/Public/file.test");
	CHECK(file4.baseName()=="file");
	CHECK(file4.directory()=="C:/Users/Public/");
	CHECK(file4.extension()=="test");
	CHECK(!file4.isDirectory());
	CHECK(file4.name()=="file.test");
	CHECK(file4.path()=="file.test");

	FilePath directory1("C:/Users/Public");
	CHECK(directory1.fullPath()=="C:/Users/Public");
	CHECK(directory1.baseName()=="Public");
	CHECK(directory1.directory()=="");
	CHECK(directory1.extension()=="");
	CHECK(directory1.name()=="Public");
	CHECK(directory1.path()=="C:/Users/Public");

	FilePath directory2("C:/Users/Public/");
	CHECK(directory2.fullPath()=="C:/Users/Public/");
	CHECK(directory2.baseName()=="");
	CHECK(directory2.directory()=="C:/Users/Public/");
	CHECK(directory2.extension()=="");
	CHECK(directory2.name()=="");
	CHECK(directory2.path()=="");
}

ADD_TEST(FilePathTest, CommonLinux) {

	FilePath file1("/user/dev/file.test");
	CHECK(file1.fullPath()=="/user/dev/file.test");
	CHECK(file1.baseName()=="file");
	CHECK(file1.directory()=="");
	CHECK(file1.extension()=="test");
	CHECK(!file1.isDirectory());
	CHECK(file1.name()=="file.test");
	CHECK(file1.path()=="/user/dev/file.test");

	FilePath file2("/user/dev", "file.test");
	CHECK(file2.fullPath()=="/user/dev/file.test");
	CHECK(file2.baseName()=="file");
	CHECK(file2.directory()=="/user/dev/");
	CHECK(file2.extension()=="test");
	CHECK(!file2.isDirectory());
	CHECK(file2.name()=="file.test");
	CHECK(file2.path()=="file.test");
}

ADD_TEST(FilePathTest, Operations) {

	FilePath file("/user/dev/file.test");
	CHECK(file.fullPath()=="/user/dev/file.test");

	file.set("/user/dev/file2.test");
	CHECK(file.fullPath()=="/user/dev/file2.test");

	file.appendDirectory("/root");
	CHECK(file.fullPath()=="/root/user/dev/file2.test");

	file.set("/user/dev/");
	CHECK(file.fullPath()=="/user/dev/");
	file.appendPath("file.test");
	CHECK(file.fullPath()=="/user/dev/file.test");
	CHECK(file.baseName()=="file");
	CHECK(file.directory()=="/user/dev/");
	CHECK(file.extension()=="test");
	CHECK(file.name()=="file.test");
	CHECK(file.path()=="file.test");

	file.set("/user", "dev");
	CHECK(file.fullPath()=="/user/dev");
	CHECK(file.baseName()=="dev");
	CHECK(file.directory()=="/user/");
	CHECK(file.extension()=="");
	CHECK(file.name()=="dev");
	CHECK(file.path()=="dev");

	FilePath file2("/tmp", "tmpFile.txt");
	CHECK(file2.fullPath()=="/tmp/tmpFile.txt");
	CHECK(file2.baseName()=="tmpFile");
	CHECK(file2.directory()=="/tmp/");
	CHECK(file2.extension()=="txt");
	CHECK(file2.name()=="tmpFile.txt");
	CHECK(file2.path()=="tmpFile.txt");
	CHECK(!file2.isDirectory());

	file = file2;
	CHECK(file.fullPath()=="/tmp/tmpFile.txt");
	CHECK(file.baseName()=="tmpFile");
	CHECK(file.directory()=="/tmp/");
	CHECK(file.extension()=="txt");
	CHECK(file.name()=="tmpFile.txt");
	CHECK(file.path()=="tmpFile.txt");

	file.setDirectory("/root", "/user/");
	CHECK(file.fullPath()=="/root/user/tmpFile.txt");
	CHECK(file.baseName()=="tmpFile");
	CHECK(file.directory()=="/root/user/");
	CHECK(file.extension()=="txt");
	CHECK(file.name()=="tmpFile.txt");
	CHECK(file.path()=="tmpFile.txt");

	file.setPath("tmp", "/file.test");
	CHECK(file.fullPath()=="/root/user/tmp/file.test");
	CHECK(file.baseName()=="file");
	CHECK(file.directory()=="/root/user/");
	CHECK(file.extension()=="test");
	CHECK(file.name()=="file.test");
	CHECK(file.path()=="tmp/file.test");
}
