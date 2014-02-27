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

#include "LUABroadcaster.h"
#include "LUAServer.h"
#include "Servers.h"

using namespace std;
using namespace Mona;

void LUABroadcaster::Init(lua_State *pState, Broadcaster& broadcaster) {
	Script::Collection<Broadcaster, LUABroadcaster>(pState, -1, "|items", 0, &broadcaster);
	lua_pop(pState, 1);
}

void LUABroadcaster::AddServer(lua_State* pState, Broadcaster& broadcaster, const string& address) {
	// -1 must be the server table!
	if (Script::FromObject(pState, broadcaster)) {
		Script::Collection(pState, -1, "|items", broadcaster.count());
		lua_pushstring(pState, address.c_str());
		lua_pushvalue(pState, -4);
		lua_rawset(pState, -3); // rawset cause NewIndexProhibited
		lua_pop(pState, 2);
	}
}
void LUABroadcaster::RemoveServer(lua_State* pState, Broadcaster& broadcaster, const string& address) {
	if (Script::FromObject(pState, broadcaster)) {
		Script::Collection(pState, -1, "|items", broadcaster.count());
		lua_pushstring(pState, address.c_str());
		lua_pushnil(pState);
		lua_rawset(pState, -3); // rawset cause NewIndexProhibited
		lua_pop(pState, 2);
	}
}

int LUABroadcaster::Item(lua_State *pState) {
	// 1 => clients table
	// 2 => parameter
	if (!lua_isstring(pState, 2))
		return 0;
	Broadcaster* pBroadcaster = Script::GetCollector<Broadcaster>(pState, 1);
	if (!pBroadcaster)
		return 0;
	ServerConnection* pServer = NULL;
	if (lua_isnumber(pState,2)) {
		UInt32 index = (UInt32)lua_tonumber(pState, 2);
		if (index>0)
			pServer = (*pBroadcaster)[--index];
	} else
		pServer = (*pBroadcaster)[lua_tostring(pState, 2)];
	SCRIPT_BEGIN(pState)
		if (pServer)
			SCRIPT_ADD_OBJECT(ServerConnection, LUAServer, *pServer)
	SCRIPT_END
	return pServer ? 1 : 0;
}

int LUABroadcaster::Broadcast(lua_State* pState) {
	SCRIPT_CALLBACK(Broadcaster,broadcaster)
		string handler(SCRIPT_READ_STRING(""));
		if(handler.empty() || handler==".") {
			ERROR("handler of one sending server message can't be null or equal to '.'")
		} else {
			ServerMessage message(broadcaster.poolBuffers);
			SCRIPT_READ_DATA(message)
			broadcaster.broadcast(handler,message);
		}
	SCRIPT_CALLBACK_RETURN
}

int LUABroadcaster::Get(lua_State *pState) {
	SCRIPT_CALLBACK(Broadcaster,broadcaster)
		const char* name = SCRIPT_READ_STRING("");
		if (strcmp(name, "broadcast") == 0)
			SCRIPT_WRITE_FUNCTION(&LUABroadcaster::Broadcast)
		else if (strcmp(name, "initiators")==0) {
			Servers* pServers = dynamic_cast<Servers*>(&broadcaster);
			if (pServers)
				lua_getglobal(pState, "m.s.i");
		} else if (strcmp(name, "targets") == 0) {
			Servers* pServers = dynamic_cast<Servers*>(&broadcaster);
			if (pServers)
				lua_getglobal(pState, "m.s.t");
		}
	SCRIPT_CALLBACK_RETURN
}

int LUABroadcaster::Set(lua_State *pState) {
	SCRIPT_CALLBACK(Broadcaster,broadcaster)
		lua_rawset(pState,1); // consumes key and value
	SCRIPT_CALLBACK_RETURN
}

