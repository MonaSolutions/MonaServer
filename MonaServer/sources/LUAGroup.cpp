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

#include "LUAGroup.h"
#include "Mona/Invoker.h"
#include "Mona/Util.h"
#include "LUAClient.h"

using namespace std;
using namespace Mona;

void LUAGroup::Init(lua_State* pState, Group& group) {
	// just for collector
	Script::Collection<LUAGroup>(pState, -1, "members");
	lua_setfield(pState, -2, "members");

	lua_getmetatable(pState, -1);
	string hex;
	lua_pushstring(pState, Mona::Util::FormatHex(group.id, ID_SIZE, hex).c_str());
	lua_pushvalue(pState, -1);
	lua_setfield(pState, -3,"|id");
	lua_replace(pState, -2);
	lua_setfield(pState, -2, "id");
}

void LUAGroup::AddClient(lua_State* pState, int indexGroup) {
	// -1 must be the client table!
	Script::Collection(pState, indexGroup, "members");
	lua_getmetatable(pState, -2);
	lua_getfield(pState, -1, "|id");
	lua_replace(pState, -2);
	lua_pushvalue(pState, -3);
	Script::FillCollection(pState,1);
	lua_pop(pState, 1);
}

void LUAGroup::RemoveClient(lua_State* pState, Client& client) {
	// -1 must be the group table!
	if (!Script::FromObject<Client>(pState, client))
		return;
	Script::Collection(pState, -2, "members");
	lua_getmetatable(pState, -2);
	lua_getfield(pState, -1, "|id");
	lua_replace(pState, -2);
	lua_pushnil(pState);
	Script::FillCollection(pState,1);
	lua_pop(pState, 2);
}

int LUAGroup::Item(lua_State *pState) {
	SCRIPT_CALLBACK(Group,group)

		if (lua_isstring(pState, 2)) {
			SCRIPT_READ_BINARY(id,size)
			Client* pMember(NULL);
			UInt8 rawId[32];
			id = Script::ToRawId(id, size, rawId);
			if (id && (pMember = group(id)))
				Script::AddObject<LUAClient>(pState,*pMember);
		}

	SCRIPT_CALLBACK_RETURN
}


int	LUAGroup::Size(lua_State* pState) {
	SCRIPT_CALLBACK(Group,group)
		SCRIPT_WRITE_NUMBER(group.count())
	SCRIPT_CALLBACK_RETURN
}

int LUAGroup::Get(lua_State *pState) {
	SCRIPT_CALLBACK(Group,group)
		const char* name = SCRIPT_READ_STRING(NULL);
		if (name) {
			if(strcmp(name,"id")==0) {
				lua_getmetatable(pState, 1);
				lua_getfield(pState, -1, "|id");
				lua_replace(pState, -2);
				SCRIPT_FIX_RESULT
			} else if (strcmp(name, "rawId") == 0) {
				SCRIPT_WRITE_BINARY(group.id,ID_SIZE);
				SCRIPT_FIX_RESULT
			} else if (strcmp(name, "size") == 0) {
				SCRIPT_WRITE_FUNCTION(LUAGroup::Size)
				SCRIPT_FIX_RESULT
			} else if (strcmp(name, "members") == 0) {
				Script::Collection(pState, 1, "members");
				SCRIPT_FIX_RESULT
			}
		}
	SCRIPT_CALLBACK_RETURN
}

int LUAGroup::Set(lua_State *pState) {
	SCRIPT_CALLBACK(Group,group)
		lua_rawset(pState,1); // consumes key and value
	SCRIPT_CALLBACK_RETURN
}




