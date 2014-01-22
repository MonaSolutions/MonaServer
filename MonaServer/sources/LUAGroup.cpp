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
#include "LUAClient.h"
#include "Mona/Invoker.h"
#include "Mona/Util.h"

using namespace std;
using namespace Mona;


void LUAGroup::AddClient(lua_State* pState, Group& group, Client& client, int indexGroup,int indexClient) {
	Script::Collection(pState, indexGroup, "|items", group.size());
	LUAClient::GetID(pState, client);
	lua_pushvalue(pState, indexClient);
	lua_rawset(pState, -3);
	if (!client.name.empty()) {
		lua_pushstring(pState, client.name.c_str());
		lua_pushvalue(pState, indexClient);
		lua_rawset(pState, -3); // rawset cause NewIndexProhibited
	}
	lua_pop(pState, 1);
}

void LUAGroup::RemoveClient(lua_State* pState, Group& group, Client& client, int indexGroup) {
	Script::Collection(pState, indexGroup, "|items", group.size());
	LUAClient::GetID(pState, client);
	lua_pushnil(pState);
	lua_rawset(pState, -3);
	if (!client.name.empty()) {
		lua_pushstring(pState, client.name.c_str());
		lua_pushnil(pState);
		lua_rawset(pState, -3); // rawset cause NewIndexProhibited
	}
	lua_pop(pState, 1);
}

int LUAGroup::Item(lua_State *pState) {
	// 1 => groups table
	// 2 => parameter
	if (!lua_isstring(pState, 2))
		return 0;
	Invoker* pInvoker = Script::GetCollector<Invoker>(pState,1);
	if (!pInvoker)
		return 0;
	Group* pGroup(NULL);
	UInt32 size = lua_objlen(pState, 2);
	const UInt8* id = (const UInt8*)lua_tostring(pState, 2);
	if (size == ID_SIZE)
		pGroup = pInvoker->groups(id);
	else if (size == (ID_SIZE * 2))
		pGroup = pInvoker->groups(Util::UnformatHex((UInt8*)id, size));
	SCRIPT_BEGIN(pState)
		if (pGroup)
			SCRIPT_ADD_OBJECT(Group, LUAGroup, *pGroup)
	SCRIPT_END
	return pGroup ? 1 : 0;
}

int LUAGroup::Get(lua_State *pState) {
	SCRIPT_CALLBACK(Group,group)
		const char* name = SCRIPT_READ_STRING("");
		if(strcmp(name,"id")==0) {
			if (lua_getmetatable(pState, 1)) {
				lua_getfield(pState, -1, "|id");
				if (!lua_isstring(pState, -1)) {
					lua_pop(pState, 1);
					string hex;
					lua_pushstring(pState, Mona::Util::FormatHex(group.id, ID_SIZE, hex).c_str());
					lua_setfield(pState, -2,"|id");
				}
				lua_replace(pState, -2);
			}
		} else if (strcmp(name, "rawId") == 0) {
			SCRIPT_WRITE_BINARY(group.id,ID_SIZE);
		} else if (strcmp(name, "size") == 0) {
			SCRIPT_WRITE_NUMBER(group.size());
		}
	SCRIPT_CALLBACK_RETURN
}

int LUAGroup::Set(lua_State *pState) {
	SCRIPT_CALLBACK(Group,group)
		lua_rawset(pState,1); // consumes key and value
	SCRIPT_CALLBACK_RETURN
}




