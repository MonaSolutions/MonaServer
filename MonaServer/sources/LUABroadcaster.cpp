/* 
	Copyright 2013 Mona - mathieu.poux[a]gmail.com
 
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
#include "Broadcaster.h"
#include "LUAServer.h"

using namespace std;
using namespace Mona;


const char*		LUABroadcaster::Name="LUABroadcaster";

int LUABroadcaster::IPairs(lua_State* pState) {
	SCRIPT_CALLBACK(Broadcaster,LUABroadcaster,broadcaster)
		lua_getglobal(pState,"next");
		if(!lua_iscfunction(pState,-1))
			SCRIPT_ERROR("'next' should be a LUA function, it should not be overloaded")
		else {
			lua_newtable(pState);
			UInt32 i=0;
			for(ServerConnection* pServer : broadcaster) {
				SCRIPT_WRITE_PERSISTENT_OBJECT(ServerConnection, LUAServer, (*pServer))
				lua_rawseti(pState,-2,++i);
			}
		}
	SCRIPT_CALLBACK_RETURN
}

int LUABroadcaster::Broadcast(lua_State* pState) {
	SCRIPT_CALLBACK(Broadcaster,LUABroadcaster,broadcaster)
		string handler(SCRIPT_READ_STRING(""));
		if(handler.empty() || handler==".") {
			ERROR("handler of one sending server message can't be null or equal to '.'")
		} else {
			ServerMessage message;
			SCRIPT_READ_DATA(message)
			broadcaster.broadcast(handler,message);
		}
	SCRIPT_CALLBACK_RETURN
}

int LUABroadcaster::Get(lua_State *pState) {
	SCRIPT_CALLBACK(Broadcaster,LUABroadcaster,broadcaster)
		string name = SCRIPT_READ_STRING("");
		if(name=="ipairs")
			SCRIPT_WRITE_FUNCTION(&LUABroadcaster::IPairs)
		else if(name == "count")
			SCRIPT_WRITE_NUMBER(broadcaster.count())
		else if(name == "broadcast")
			SCRIPT_WRITE_FUNCTION(&LUABroadcaster::Broadcast)
		else if(name=="(") {
			ServerConnection* pServer = NULL;
			if(SCRIPT_NEXT_TYPE==LUA_TNUMBER) {
				UInt32 index = SCRIPT_READ_UINT(0);
				if(index>0)
					pServer = broadcaster[--index];
			} else
				pServer = broadcaster[SCRIPT_READ_STRING("")];
			if(pServer)
				SCRIPT_WRITE_PERSISTENT_OBJECT(ServerConnection,LUAServer,*pServer)
		}
	SCRIPT_CALLBACK_RETURN
}

int LUABroadcaster::Set(lua_State *pState) {
	SCRIPT_CALLBACK(Broadcaster,LUABroadcaster,broadcaster)
		string name = SCRIPT_READ_STRING("");
		lua_rawset(pState,1); // consumes key and value
	SCRIPT_CALLBACK_RETURN
}

