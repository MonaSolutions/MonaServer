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

#include "LUAFile.h"

using namespace std;
using namespace Mona;


int LUAFile::Get(lua_State *pState) {
	SCRIPT_CALLBACK(File,file)
		const char* name = SCRIPT_READ_STRING(NULL);
		
		if (name) {
			if(strcmp(name,"name")==0) {
				SCRIPT_WRITE_STRING(file.name().c_str())
				SCRIPT_FIX_RESULT
			} else if(strcmp(name,"baseName")==0) {
				SCRIPT_WRITE_STRING(file.baseName().c_str())
				SCRIPT_FIX_RESULT
			} else if(strcmp(name,"parent")==0) {
				SCRIPT_WRITE_STRING(file.parent().c_str())
				SCRIPT_FIX_RESULT
			} else if(strcmp(name,"extension")==0) {
				SCRIPT_WRITE_STRING(file.extension().c_str())
				SCRIPT_FIX_RESULT
			} else if(strcmp(name,"size")==0) {
				SCRIPT_WRITE_NUMBER(file.size())
				SCRIPT_FIX_RESULT
			} else if(strcmp(name,"lastModified")==0) {
				SCRIPT_WRITE_NUMBER(file.lastModified())
				SCRIPT_FIX_RESULT
			} else if (strcmp(name,"isFolder")==0) {
				SCRIPT_WRITE_BOOL(file.isFolder())
				SCRIPT_FIX_RESULT
			} else if (strcmp(name,"isAbsolute")==0) {
				SCRIPT_WRITE_BOOL(file.isAbsolute())
				SCRIPT_FIX_RESULT
			} else if (strcmp(name,"path")==0) {
				SCRIPT_WRITE_STRING(file.path().c_str());
				SCRIPT_FIX_RESULT
			}
		}
	SCRIPT_CALLBACK_RETURN
}

int LUAFile::Set(lua_State *pState) {
	SCRIPT_CALLBACK(File,file)
		lua_rawset(pState,1); // consumes key and value
	SCRIPT_CALLBACK_RETURN
}