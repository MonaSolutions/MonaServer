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

#include "LUAPath.h"

using namespace std;
using namespace Mona;


int LUAPath::Get(lua_State *pState) {
	SCRIPT_CALLBACK(Path,path)
		const char* name = SCRIPT_READ_STRING(NULL);
		
		if (name) {
			if(strcmp(name,"name")==0) {
				SCRIPT_WRITE_STRING(path.name().c_str())
				SCRIPT_CALLBACK_FIX_INDEX
			} else if(strcmp(name,"baseName")==0) {
				SCRIPT_WRITE_STRING(path.baseName().c_str())
				SCRIPT_CALLBACK_FIX_INDEX
			} else if(strcmp(name,"parent")==0) {
				SCRIPT_WRITE_STRING(path.parent().c_str())
				SCRIPT_CALLBACK_FIX_INDEX
			} else if(strcmp(name,"extension")==0) {
				SCRIPT_WRITE_STRING(path.extension().c_str())
				SCRIPT_CALLBACK_FIX_INDEX
			} else if(strcmp(name,"size")==0) {
				SCRIPT_WRITE_NUMBER(path.size())
				SCRIPT_CALLBACK_FIX_INDEX
			} else if(strcmp(name,"lastModified")==0) {
				SCRIPT_WRITE_NUMBER(path.lastModified())
				SCRIPT_CALLBACK_FIX_INDEX
			} else if (strcmp(name,"isDirectory")==0) {
				SCRIPT_WRITE_BOOL(path.isDirectory())
				SCRIPT_CALLBACK_FIX_INDEX
			} else if (strcmp(name,"value")==0) {
				SCRIPT_WRITE_STRING(path.toString().c_str());
				SCRIPT_CALLBACK_FIX_INDEX
			}
		}
	SCRIPT_CALLBACK_RETURN
}

int LUAPath::Set(lua_State *pState) {
	SCRIPT_CALLBACK(Path,path)
		lua_rawset(pState,1); // consumes key and value
	SCRIPT_CALLBACK_RETURN
}