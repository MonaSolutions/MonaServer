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

#include "LUAMember.h"
#include "Mona/Util.h"

using namespace Mona;
using namespace std;

void LUAMember::Init(lua_State* pState, Peer& member) {
	lua_getmetatable(pState, -1);
	string hex;
	lua_pushstring(pState, Mona::Util::FormatHex(member.id, ID_SIZE, hex).c_str());
	lua_pushvalue(pState, -1);
	lua_setfield(pState, -3,"|id");
	lua_replace(pState, -2);
	lua_setfield(pState, -2, "id");
}

int LUAMember::Destroy(lua_State* pState) {
	SCRIPT_DESTRUCTOR_CALLBACK(Peer,member)
		delete &member;
	SCRIPT_CALLBACK_RETURN
}

int LUAMember::Release(lua_State* pState) {
	SCRIPT_CALLBACK(Peer,member)
		member.unsubscribeGroups();
	SCRIPT_CALLBACK_RETURN
}

int LUAMember::Get(lua_State *pState) {
	SCRIPT_CALLBACK(Peer,member)
		const char* name = SCRIPT_READ_STRING(NULL);
		if(name) {
			if (strcmp(name,"id")==0) {
				lua_getmetatable(pState, 1);
				lua_getfield(pState, -1, "|id");
				lua_replace(pState, -2);
				SCRIPT_CALLBACK_FIX_INDEX(name)
			} else if (strcmp(name, "rawId") == 0) {
				SCRIPT_WRITE_BINARY(member.id,ID_SIZE)
				SCRIPT_CALLBACK_FIX_INDEX(name)
			} else if (strcmp(name, "release") == 0) {
				SCRIPT_WRITE_FUNCTION(LUAMember::Release);
				SCRIPT_CALLBACK_FIX_INDEX(name)
			}
		}
	SCRIPT_CALLBACK_RETURN
}

int LUAMember::Set(lua_State *pState) {
	SCRIPT_CALLBACK(Peer,member)
		lua_rawset(pState,1); // consumes key and value
	SCRIPT_CALLBACK_RETURN
}
