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

#include "LUAFilePath.h"

using namespace std;
using namespace Mona;

void LUAFiles::Init(lua_State* pState, LUAFiles& files) {
}

int	LUAFiles::Destroy(lua_State* pState) {
	SCRIPT_DESTRUCTOR_CALLBACK(LUAFiles, files)
		delete &files;
	SCRIPT_CALLBACK_RETURN
}

int LUAFiles::Get(lua_State *pState) {
	SCRIPT_CALLBACK(LUAFiles, filepath)
		const char* name = SCRIPT_READ_STRING("");
	SCRIPT_CALLBACK_RETURN
}

int LUAFiles::Set(lua_State *pState) {
	SCRIPT_CALLBACK(LUAFiles, files)
		const char* name = SCRIPT_READ_STRING("");
		lua_rawset(pState,1); // consumes key and value
	SCRIPT_CALLBACK_RETURN
}


void LUAFilePath::Init(lua_State* pState, LUAFilePath& filepath) {}

int	LUAFilePath::Destroy(lua_State* pState) {
	SCRIPT_DESTRUCTOR_CALLBACK(LUAFilePath, filepath)
		delete &filepath;
	SCRIPT_CALLBACK_RETURN
}

int LUAFilePath::Get(lua_State *pState) {
	SCRIPT_CALLBACK(LUAFilePath, filepath)
		const char* name = SCRIPT_READ_STRING("");

		if(strcmp(name,"fullPath")==0) {
			SCRIPT_WRITE_STRING(filepath._path.fullPath().c_str())
		}  else if(strcmp(name,"name")==0) {
			SCRIPT_WRITE_STRING(filepath._path.name().c_str())
		} else if(strcmp(name,"baseName")==0) {
			SCRIPT_WRITE_STRING(filepath._path.baseName().c_str())
		} else if(strcmp(name,"extension")==0) {
			SCRIPT_WRITE_STRING(filepath._path.extension().c_str())
		} else if(strcmp(name,"size")==0) {
			SCRIPT_WRITE_NUMBER(filepath._path.size())
		} else if(strcmp(name,"lastModified")==0) {
			SCRIPT_WRITE_NUMBER(filepath._path.lastModified())
		} else if (strcmp(name,"isDirectory")==0) {
			SCRIPT_WRITE_BOOL(filepath._path.isDirectory())
		}
	SCRIPT_CALLBACK_RETURN
}

int LUAFilePath::Set(lua_State *pState) {
	SCRIPT_CALLBACK(LUAFilePath, filepath)
		const char* name = SCRIPT_READ_STRING("");
		
		lua_rawset(pState,1); // consumes key and value
	SCRIPT_CALLBACK_RETURN
}