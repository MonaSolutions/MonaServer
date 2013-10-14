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

#include "LUAServers.h"
#include "Servers.h"
#include "LUAServer.h"
#include "LUABroadcaster.h"

using namespace std;
using namespace Mona;
using namespace Poco;

const char*		LUAServers::Name="LUAServers";

int LUAServers::IPairs(lua_State* pState) {
	SCRIPT_CALLBACK(Servers,LUAServers,servers)
		lua_getglobal(pState,"next");
		if(!lua_iscfunction(pState,-1))
			SCRIPT_ERROR("'next' should be a LUA function, it should not be overloaded")
		else {
			lua_newtable(pState);
			Servers::Iterator it;
			Mona::UInt32 i=0;
			for(it=servers.begin();it!=servers.end();++it) {
				SCRIPT_WRITE_PERSISTENT_OBJECT(ServerConnection,LUAServer,(**it))
				lua_rawseti(pState,-2,++i);
			}
		}
	SCRIPT_CALLBACK_RETURN
}

int LUAServers::Broadcast(lua_State* pState) {
	SCRIPT_CALLBACK(Servers,LUAServers,servers)
		string handler(SCRIPT_READ_STRING(""));
		if(handler.empty() || handler==".") {
			ERROR("handler of one sending server message can't be null or equal to '.'")
		} else {
			ServerMessage message;
			SCRIPT_READ_DATA(message)
			servers.broadcast(handler,message);
		}
	SCRIPT_CALLBACK_RETURN
}

int LUAServers::Get(lua_State *pState) {
	SCRIPT_CALLBACK(Servers,LUAServers,servers)
		string name = SCRIPT_READ_STRING("");
		if(name=="ipairs")
			SCRIPT_WRITE_FUNCTION(&LUAServers::IPairs)
		else if(name == "count")
			SCRIPT_WRITE_NUMBER(servers.count())
		else if(name == "broadcast")
			SCRIPT_WRITE_FUNCTION(&LUAServers::Broadcast)
		else if(name == "initiators")
			SCRIPT_WRITE_PERSISTENT_OBJECT(Broadcaster,LUABroadcaster,servers.initiators)
		else if(name == "targets")
			SCRIPT_WRITE_PERSISTENT_OBJECT(Broadcaster,LUABroadcaster,servers.targets)
		else if(name=="(") {
			ServerConnection* pServer = NULL;
			if(SCRIPT_NEXT_TYPE==LUA_TNUMBER) {
				Mona::UInt32 index = SCRIPT_READ_UINT(0);
				if(index>0)
					pServer = servers[--index];
			} else
				pServer = servers[SCRIPT_READ_STRING("")];
			if(pServer)
				SCRIPT_WRITE_PERSISTENT_OBJECT(ServerConnection,LUAServer,*pServer)
		}
	SCRIPT_CALLBACK_RETURN
}

int LUAServers::Set(lua_State *pState) {
	SCRIPT_CALLBACK(Servers,LUAServers,servers)
		string name = SCRIPT_READ_STRING("");
		lua_rawset(pState,1); // consumes key and value
	SCRIPT_CALLBACK_RETURN
}