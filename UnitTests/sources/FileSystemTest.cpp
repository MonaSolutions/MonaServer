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
#include "Mona/File.h"
#include "Mona/Util.h"
#include <fstream>

using namespace std;
using namespace Mona;

static string Home;
static const char* CurrentDir;
static string Path;
static string Path2;
static size_t ExtPos;

ADD_TEST(FileSystemTest, Static) {
	CHECK(FileSystem::GetCurrentApp(Home) && FileSystem::GetCurrentApp());
	CHECK(!Home.empty() && FileSystem::IsAbsolute(Home));
	CHECK(FileSystem::GetCurrentDir(Home) && (CurrentDir=FileSystem::GetCurrentDir()));
	CHECK(!Home.empty() && FileSystem::IsAbsolute(Home));
	CHECK(FileSystem::GetHome(Home) && FileSystem::GetHome());
	CHECK(!Home.empty() && FileSystem::IsAbsolute(Home));
}

#define CHECK_FILE(PATH,FILE_TYPE, PARENT,NAME,EXTENSION_POS) CHECK(FileSystem::GetFile(PATH,Path,ExtPos, Path2)==FILE_TYPE && EXTENSION_POS==ExtPos && NAME==Path && PARENT==Path2);


ADD_TEST(FileSystemTest, Resolve) {

	File parent(File::CurrentDir.parent());

	// ABSOLUTE
#if defined(_WIN32)
	CHECK_FILE("C:/", FileSystem::FOLDER, "C:/", "", string::npos);
	CHECK_FILE("C:/..", FileSystem::FILE, "C:/", "", string::npos);
	CHECK_FILE("C:/../", FileSystem::FOLDER, "C:/", "", string::npos);
	CHECK_FILE("C:/../..", FileSystem::FILE, "C:/", "", string::npos);
	CHECK_FILE("C:/../../", FileSystem::FOLDER, "C:/", "", string::npos);
	CHECK_FILE("C:/../.", FileSystem::FILE, "C:/", "", string::npos);
	CHECK_FILE("C:/.././", FileSystem::FOLDER, "C:/", "", string::npos);
	CHECK_FILE("C:/../test/.", FileSystem::FILE, "C:/../", "test", string::npos);
	CHECK_FILE("C://salut.txt", FileSystem::FILE, "C:/", "salut.txt", 5);
#endif
	CHECK_FILE("/", FileSystem::FOLDER, "/", "", string::npos);
	CHECK_FILE("/..", FileSystem::FILE, "/", "", string::npos);
	CHECK_FILE("/../", FileSystem::FOLDER, "/", "", string::npos);
	CHECK_FILE("/../..", FileSystem::FILE, "/", "", string::npos);
	CHECK_FILE("/../../", FileSystem::FOLDER, "/", "", string::npos);
	CHECK_FILE("/../.", FileSystem::FILE, "/", "", string::npos);
	CHECK_FILE("/.././", FileSystem::FOLDER, "/", "", string::npos);
	CHECK_FILE("/../test/.", FileSystem::FILE, "/../", "test", string::npos);
	CHECK_FILE("/../test/./", FileSystem::FOLDER, "/../", "test", string::npos);
	CHECK_FILE("//salut.txt", FileSystem::FILE, "/", "salut.txt", 5);
	CHECK_FILE("//salut.txt/", FileSystem::FOLDER, "/", "salut.txt", 5);

	CHECK_FILE("/te/main.cpp/t", FileSystem::FILE, "/te/main.cpp/", "t", string::npos);
	CHECK_FILE("/te/main.cpp/..", FileSystem::FILE, "/", "te", string::npos);
	CHECK_FILE("/te/main.cpp/../", FileSystem::FOLDER, "/", "te", string::npos);
	CHECK_FILE("/te/main.cpp/...", FileSystem::FILE, "/te/main.cpp/", "...", 2);
	CHECK_FILE("/te/main.cpp/.../", FileSystem::FOLDER, "/te/main.cpp/", "...", 2);
	CHECK_FILE("/./main.cpp", FileSystem::FILE, "/./", "main.cpp", 4);
	CHECK_FILE("/./main.cpp/.", FileSystem::FILE, "/./", "main.cpp", 4);
	CHECK_FILE("/./main.cpp/./", FileSystem::FOLDER, "/./", "main.cpp", 4);
	CHECK_FILE("/./main.cpp/test/..", FileSystem::FILE, "/./", "main.cpp", 4);
	CHECK_FILE("/./main.cpp/test/../", FileSystem::FOLDER, "/./", "main.cpp", 4);
	CHECK_FILE("/salut.txt", FileSystem::FILE, "/", "salut.txt", 5);
	CHECK_FILE("C:/../salut", FileSystem::FILE, "C:/../", "salut", string::npos);
	CHECK_FILE("C:/../salut", FileSystem::FILE, "C:/../", "salut", string::npos);


	// RELATIVE
	CHECK_FILE("", FileSystem::FOLDER, parent.path(), File::CurrentDir.name(), string::npos);
	CHECK_FILE("C:", FileSystem::FILE, CurrentDir, "C:", string::npos);
	CHECK_FILE("salut.txt", FileSystem::FILE, CurrentDir, "salut.txt", 5);
	CHECK_FILE("../", FileSystem::FOLDER, parent.parent(), parent.name(), string::npos); 
	CHECK_FILE("..", FileSystem::FILE, parent.parent(), parent.name(), string::npos); 
	CHECK_FILE("./", FileSystem::FOLDER, parent.path(), File::CurrentDir.name(), string::npos); 
	CHECK_FILE(".", FileSystem::FILE, parent.path(), File::CurrentDir.name(), string::npos); 
	CHECK_FILE("../salut", FileSystem::FILE, "../", "salut", string::npos); 


	CHECK(!FileSystem::IsAbsolute(""));
	CHECK(!FileSystem::IsAbsolute("."));
	CHECK(!FileSystem::IsAbsolute(".."));
	CHECK(FileSystem::IsAbsolute("/."));
	CHECK(FileSystem::IsAbsolute("/.."));
	CHECK(FileSystem::IsAbsolute("/"));
	CHECK(FileSystem::IsAbsolute("/f/"));
#if defined(_WIN32)
	CHECK(FileSystem::IsAbsolute("C:/"));
#else
	CHECK(!FileSystem::IsAbsolute("C:/"));
#endif
	CHECK(!FileSystem::IsAbsolute("C:"));


	CHECK(FileSystem::MakeAbsolute(Path.assign(""))=="/");
	CHECK(FileSystem::MakeAbsolute(Path.assign("."))=="/.");
	CHECK(FileSystem::MakeAbsolute(Path.assign(".."))=="/..");
	CHECK(FileSystem::MakeAbsolute(Path.assign("f"))=="/f");
	CHECK(FileSystem::MakeAbsolute(Path.assign("/."))=="/.");
	CHECK(FileSystem::MakeAbsolute(Path.assign("/.."))=="/..");
	CHECK(FileSystem::MakeAbsolute(Path.assign("/"))=="/");
	CHECK(FileSystem::MakeAbsolute(Path.assign("/f/"))=="/f/");
#if defined(_WIN32)
	CHECK(FileSystem::MakeAbsolute(Path.assign("C:/"))=="C:/");
#else
	CHECK(FileSystem::MakeAbsolute(Path.assign("C:/"))=="/C:/");
#endif
	CHECK(FileSystem::MakeAbsolute(Path.assign("C:")) == "/C:");


	CHECK(FileSystem::MakeRelative(Path.assign(""))=="");
	CHECK(FileSystem::MakeRelative(Path.assign("."))==".");
	CHECK(FileSystem::MakeRelative(Path.assign(".."))=="..");
	CHECK(FileSystem::MakeRelative(Path.assign("f"))=="f");
	CHECK(FileSystem::MakeRelative(Path.assign("/."))==".");
	CHECK(FileSystem::MakeRelative(Path.assign("/.."))=="..");
	CHECK(FileSystem::MakeRelative(Path.assign("/"))=="");
	CHECK(FileSystem::MakeRelative(Path.assign("/f/"))=="f/");
#if defined(_WIN32)
	CHECK(FileSystem::MakeRelative(Path.assign("C:/"))=="");
#else
	CHECK(FileSystem::MakeRelative(Path.assign("C:/"))=="C:/");
#endif
	CHECK(FileSystem::MakeRelative(Path.assign("C:")) == "C:");


	CHECK(FileSystem::Resolve(Path.assign(File::CurrentDir.path())) == File::CurrentDir.path());
	CHECK(FileSystem::Resolve(Path.assign("/")) == "/");
#if defined(_WIN32)
	CHECK(FileSystem::Resolve(Path.assign("C:/")) == "C:/");
	CHECK(FileSystem::Resolve(Path.assign("C:/..")) == "C:/.");
#else
	CHECK(FileSystem::Resolve(Path.assign("C:/")) == Path2.assign(File::CurrentDir.path()).append("C:/"));
#endif
	CHECK(FileSystem::Resolve(Path.assign(".")) == FileSystem::MakeFile(Path2.assign(File::CurrentDir.path())));
	CHECK(FileSystem::Resolve(Path.assign("./")) == File::CurrentDir.path());
	CHECK(FileSystem::Resolve(Path.assign("..")) == FileSystem::MakeFile(Path2.assign(parent.path())));
	CHECK(FileSystem::Resolve(Path.assign("../")) == parent.path());
	CHECK(FileSystem::Resolve(Path.assign("../..")) == FileSystem::MakeFile(Path2.assign(parent.parent())));
	CHECK(FileSystem::Resolve(Path.assign("../../")) == parent.parent());
	CHECK(FileSystem::Resolve(Path.assign("./.././../.")) == FileSystem::MakeFile(Path2.assign(parent.parent())));
	CHECK(FileSystem::Resolve(Path.assign("./.././.././")) == parent.parent());
	CHECK(FileSystem::Resolve(Path.assign("./test/../.")) == FileSystem::MakeFile(Path2.assign(File::CurrentDir.path())));
	CHECK(FileSystem::Resolve(Path.assign("./test/.././")) == File::CurrentDir.path());
	CHECK(FileSystem::Resolve(Path.assign("/..")) == "/.");
	CHECK(FileSystem::Resolve(Path.assign("/../")) == "/");
	CHECK(FileSystem::Resolve(Path.assign("/../.")) == "/.");
	CHECK(FileSystem::Resolve(Path.assign("/.././")) == "/");
	CHECK(FileSystem::Resolve(Path.assign("/test/../../.")) == "/.");
	CHECK(FileSystem::Resolve(Path.assign("/test/../.././")) == "/");
	CHECK(FileSystem::Resolve(Path.assign("/test/../../hi.txt")) == "/hi.txt");

	CHECK(FileSystem::IsFolder(""));
	CHECK(FileSystem::IsFolder("/"));
	CHECK(!FileSystem::IsFolder("."));
	CHECK(FileSystem::IsFolder("./"));
	CHECK(!FileSystem::IsFolder(".."));
	CHECK(FileSystem::IsFolder("../"));
	CHECK(FileSystem::IsFolder("/./"));
	CHECK(!FileSystem::IsFolder("/."));
	CHECK(FileSystem::IsFolder("/../"));
	CHECK(!FileSystem::IsFolder("/.."));
	CHECK(FileSystem::IsFolder("/f/"));
	CHECK(FileSystem::IsFolder("C:/"));
	CHECK(!FileSystem::IsFolder("C:"));
	CHECK(FileSystem::IsFolder("\\"));
	CHECK(!FileSystem::IsFolder("\\."));
	CHECK(FileSystem::IsFolder("./C:/"));
	CHECK(!FileSystem::IsFolder("C:/."));

	CHECK(FileSystem::MakeFolder(Path.assign("")).empty());
	CHECK(FileSystem::MakeFolder(Path.assign("/"))=="/");
	CHECK(FileSystem::MakeFolder(Path.assign("."))=="./");
	CHECK(FileSystem::MakeFolder(Path.assign("./"))=="./");
	CHECK(FileSystem::MakeFolder(Path.assign(".."))=="../");
	CHECK(FileSystem::MakeFolder(Path.assign("../"))=="../");
	CHECK(FileSystem::MakeFolder(Path.assign("/./"))=="/./");
	CHECK(FileSystem::MakeFolder(Path.assign("/."))=="/./");
	CHECK(FileSystem::MakeFolder(Path.assign("/../"))=="/../");
	CHECK(FileSystem::MakeFolder(Path.assign("/.."))=="/../");
	CHECK(FileSystem::MakeFolder(Path.assign("/f/"))=="/f/");
	CHECK(FileSystem::MakeFolder(Path.assign("C:/"))=="C:/");
#if defined(_WIN32)
	CHECK(FileSystem::MakeFolder(Path.assign("C:"))=="./C:/");
#else
	CHECK(FileSystem::MakeFolder(Path.assign("C:"))=="C:/");
#endif
	CHECK(FileSystem::MakeFolder(Path.assign("\\"))=="\\");
	CHECK(FileSystem::MakeFolder(Path.assign("\\."))=="\\./");

	CHECK(FileSystem::MakeFile(Path.assign(""))==".");
	CHECK(FileSystem::MakeFile(Path.assign("/"))=="/.");
	CHECK(FileSystem::MakeFile(Path.assign("."))==".");
	CHECK(FileSystem::MakeFile(Path.assign("./"))==".");
	CHECK(FileSystem::MakeFile(Path.assign(".."))=="..");
	CHECK(FileSystem::MakeFile(Path.assign("../"))=="..");
	CHECK(FileSystem::MakeFile(Path.assign("/./"))=="/.");
	CHECK(FileSystem::MakeFile(Path.assign("/."))=="/.");
	CHECK(FileSystem::MakeFile(Path.assign("/../"))=="/..");
	CHECK(FileSystem::MakeFile(Path.assign("/.."))=="/..");
	CHECK(FileSystem::MakeFile(Path.assign("/f/"))=="/f");
#if defined(_WIN32)
	CHECK(FileSystem::MakeFile(Path.assign("C:/"))=="C:/.");
#else
	CHECK(FileSystem::MakeFile(Path.assign("C:/"))=="C:");
#endif
	CHECK(FileSystem::MakeFile(Path.assign("C:"))=="C:");
	CHECK(FileSystem::MakeFile(Path.assign("\\"))=="/.");
	CHECK(FileSystem::MakeFile(Path.assign("\\."))=="\\.");

	CHECK(FileSystem::GetParent("",Path) == parent.path());
	CHECK(FileSystem::GetParent("C:",Path) == File::CurrentDir.path());
#if defined(_WIN32)
	CHECK(FileSystem::GetParent("C:/",Path) == "C:/");
#else
	CHECK(FileSystem::GetParent("C:/",Path) == File::CurrentDir.path());
#endif
	CHECK(FileSystem::GetParent("f",Path) == File::CurrentDir.path());
	CHECK(FileSystem::GetParent("f/",Path) == File::CurrentDir.path());
	CHECK(FileSystem::GetParent("/f",Path) == "/");
	CHECK(FileSystem::GetParent("/f/",Path) == "/");
	CHECK(FileSystem::GetParent(".",Path) == parent.path());
	CHECK(FileSystem::GetParent("./",Path) == parent.path());
	CHECK(FileSystem::GetParent("..",Path) == parent.parent());
	CHECK(FileSystem::GetParent("../",Path) == parent.parent());
	CHECK(FileSystem::GetParent("...",Path) == File::CurrentDir.path());
	CHECK(FileSystem::GetParent(".../",Path) == File::CurrentDir.path());
	CHECK(FileSystem::GetParent("/.",Path) == "/");
	CHECK(FileSystem::GetParent("/./",Path) == "/");
	CHECK(FileSystem::GetParent("/..",Path) == "/");
	CHECK(FileSystem::GetParent("/../",Path) == "/");
	CHECK(FileSystem::GetParent("/...",Path) == "/");
	CHECK(FileSystem::GetParent("/.../",Path) == "/");
	CHECK(FileSystem::GetParent("/.././../test/salut/../..",Path) == "/");
	

	CHECK(FileSystem::GetExtension("",Path) == File::CurrentDir.extension());
	CHECK(FileSystem::GetExtension("f",Path).empty());
	CHECK(FileSystem::GetExtension(".",Path) == File::CurrentDir.extension());
	CHECK(FileSystem::GetExtension("..",Path) == parent.extension());
	CHECK(FileSystem::GetExtension("/.",Path).empty());
	CHECK(FileSystem::GetExtension("/..",Path).empty());
	CHECK(FileSystem::GetExtension("/...",Path).empty());
	CHECK(FileSystem::GetExtension("f.e",Path)=="e");
	CHECK(FileSystem::GetExtension(".e",Path)=="e");
	CHECK(FileSystem::GetExtension("..e",Path)=="e");
	CHECK(FileSystem::GetExtension("/.e",Path)=="e");
	CHECK(FileSystem::GetExtension("/..e",Path)=="e");
	CHECK(FileSystem::GetExtension("f.e/",Path)=="e");
	CHECK(FileSystem::GetExtension(".e/",Path)=="e");
	CHECK(FileSystem::GetExtension("..e/",Path)=="e");
	CHECK(FileSystem::GetExtension("/.e/",Path)=="e");
	CHECK(FileSystem::GetExtension("/..e/",Path)=="e");
	CHECK(FileSystem::GetExtension("C:/",Path).empty());

	CHECK(FileSystem::GetName("",Path) == File::CurrentDir.name());
	CHECK(FileSystem::GetName("f",Path)=="f");
	CHECK(FileSystem::GetName(".",Path) == File::CurrentDir.name());
	CHECK(FileSystem::GetName("..",Path) == parent.name());
	CHECK(FileSystem::GetName("/.",Path).empty());
	CHECK(FileSystem::GetName("/..",Path).empty());
	CHECK(FileSystem::GetName("/...",Path)=="...");
	CHECK(FileSystem::GetName("f.e",Path)=="f.e");
	CHECK(FileSystem::GetName(".e",Path)==".e");
	CHECK(FileSystem::GetName("..e",Path)=="..e");
	CHECK(FileSystem::GetName("/.e",Path)==".e");
	CHECK(FileSystem::GetName("/..e",Path)=="..e");
	CHECK(FileSystem::GetName("f.e/",Path)=="f.e");
	CHECK(FileSystem::GetName(".e/",Path)==".e");
	CHECK(FileSystem::GetName("..e/",Path)=="..e");
	CHECK(FileSystem::GetName("/.e/",Path)==".e");
	CHECK(FileSystem::GetName("/..e/",Path)=="..e");
#if defined(_WIN32)
	CHECK(FileSystem::GetName("C:/",Path).empty());
#else
	CHECK(FileSystem::GetName("C:/",Path)=="C:");
#endif

	CHECK(FileSystem::GetBaseName("",Path) == File::CurrentDir.baseName());
	CHECK(FileSystem::GetBaseName("f",Path)=="f");
	CHECK(FileSystem::GetBaseName(".",Path) == File::CurrentDir.baseName());
	CHECK(FileSystem::GetBaseName("..",Path) == parent.baseName());
	CHECK(FileSystem::GetBaseName("/.",Path).empty());
	CHECK(FileSystem::GetBaseName("/..",Path).empty());
	CHECK(FileSystem::GetBaseName("/...",Path)=="..");
	CHECK(FileSystem::GetBaseName("f.e",Path)=="f");
	CHECK(FileSystem::GetBaseName(".e",Path).empty());
	CHECK(FileSystem::GetBaseName("..e",Path)==".");
	CHECK(FileSystem::GetBaseName("/.e",Path).empty());
	CHECK(FileSystem::GetBaseName("/..e",Path)==".");
	CHECK(FileSystem::GetBaseName("f.e/",Path)=="f");
	CHECK(FileSystem::GetBaseName(".e/",Path).empty());
	CHECK(FileSystem::GetBaseName("..e/",Path)==".");
	CHECK(FileSystem::GetBaseName("/.e/",Path).empty());
	CHECK(FileSystem::GetBaseName("/..e/",Path)==".");
#if defined(_WIN32)
	CHECK(FileSystem::GetBaseName("C:/",Path).empty());
#else
	CHECK(FileSystem::GetName("C:/",Path)=="C:");
#endif

#if defined(_WIN32)
	Util::Environment().getString("PATH", Path.assign("/Windows/System32"));
	CHECK(FileSystem::ResolveFileWithPaths(Path, Path2.assign("notepad.exe")));
#else
	Util::Environment().getString("PATH", Path.assign("/bin"));
	CHECK(FileSystem::ResolveFileWithPaths(Path, Path2.assign("ls")));
#endif

}

ADD_TEST(FileSystemTest, Creation) {
	Path.assign(Home).append(".MonaFileSystemTest/");
	Path2.assign(Path).append("/SubFolder/Folder/");

	Exception ex;
	CHECK(FileSystem::CreateDirectory(ex,Path2,FileSystem::HEAVY) && !ex);
	CHECK(FileSystem::Exists(Path2));

	CHECK(!FileSystem::Delete(ex,Path) && ex);
	CHECK(FileSystem::Exists(Path));
	ex = NULL;

	CHECK(FileSystem::Delete(ex,Path,FileSystem::HEAVY) && !ex);
	CHECK(!FileSystem::Exists(Path));

	CHECK(FileSystem::CreateDirectory(ex,Path) && !ex);
	CHECK(FileSystem::Exists(Path));
	CHECK(FileSystem::MakeFile(Path).back() == 't');
	CHECK(!FileSystem::Exists(Path));

	Path2.assign(Path).append("s/");
	if (FileSystem::Exists(Path2))
		CHECK(FileSystem::Delete(ex,Path2,FileSystem::HEAVY) && !ex);
	CHECK(FileSystem::Rename(Path, Path2));
	CHECK(!FileSystem::Exists(Path));
	CHECK(FileSystem::Exists(Path2));

	{
		ofstream ostr(Path2.append("test.txt"));
		ostr.put('t');
		ostr.close();
	}
	CHECK(FileSystem::Delete(ex,FileSystem::MakeFile(Path2)) && !ex);
}

ADD_TEST(FileSystemTest, Attributes) {
	FileSystem::Attributes attributes;
	CHECK(!FileSystem::GetAttributes(Path.assign(Home).append(".MonaFileSystemTests"), attributes) && !attributes && !attributes.lastModified);
	CHECK(FileSystem::GetAttributes(Path.append("/"), attributes) && attributes && attributes.lastModified);

	Exception ex;
#if defined(_WIN32)
	String::Append(Path, L"Приве́т नमस्ते שָׁלוֹם");
#else
	String::Append(Path, "Приве́т नमस्ते שָׁלוֹם");
#endif
	
	// file
#if defined(_WIN32)
	wchar_t wFile[_MAX_PATH];
	MultiByteToWideChar(CP_UTF8, 0, Path.c_str(), -1, wFile, _MAX_PATH);
	ofstream ostr(wFile);
#else
	ofstream ostr(Path.c_str());
#endif
	ostr.put('t');
	ostr.close();

	CHECK(FileSystem::GetAttributes(Path, attributes) && attributes && attributes.size==1)
	CHECK(FileSystem::Delete(ex, Path) && !ex);
	CHECK(!FileSystem::GetAttributes(Path, attributes) && !attributes && attributes.size==0)

	// folder
	Path += '/';
	if (FileSystem::Exists(Path))
		CHECK(FileSystem::Delete(ex, Path) && !ex);
	CHECK(FileSystem::CreateDirectory(ex,Path) && !ex);
	CHECK(FileSystem::Exists(Path))
	CHECK(FileSystem::Delete(ex, Path) && !ex);
	CHECK(!FileSystem::Exists(Path))
}

ADD_TEST(FileSystemTest, Deletion) {
	Exception ex;
	CHECK(FileSystem::Delete(ex,Path.assign(Home).append(".MonaFileSystemTests"),FileSystem::HEAVY) && !ex);
}

