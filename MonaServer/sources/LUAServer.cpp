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

#include "LUAServer.h"

using namespace std;
using namespace Mona;


int LUAServer::Send(lua_State* pState) {
	SCRIPT_CALLBACK(ServerConnection,server)
		string handler(SCRIPT_READ_STRING(""));
		if(handler.empty() || handler==".") {
			ERROR("handler of one sending server message can't be null or equal to '.'")
		} else {
			ServerMessage message(server.poolBuffers());
			SCRIPT_READ_DATA(message)
			server.send(handler,message);
		}
	SCRIPT_CALLBACK_RETURN
}

int LUAServer::Port(lua_State* pState) {
	SCRIPT_CALLBACK(ServerConnection,server)
		SCRIPT_WRITE_INT(server.port(SCRIPT_READ_STRING("")))
	SCRIPT_CALLBACK_RETURN
}


int LUAServer::Get(lua_State* pState) {
	SCRIPT_CALLBACK(ServerConnection,server)
		string name = SCRIPT_READ_STRING("");
		if(name=="send") {
			SCRIPT_WRITE_FUNCTION(&LUAServer::Send)
		} else if(name=="host") {
			SCRIPT_WRITE_STRING(server.host.c_str())
		} else if(name=="isTarget") {
			SCRIPT_WRITE_BOOL(server.isTarget)
		} else if(name=="address") {
			SCRIPT_WRITE_STRING(server.address.toString().c_str())
		} else if(name=="port") {
			SCRIPT_WRITE_FUNCTION(&LUAServer::Port)
		} else if (name == "parameters") {
			if(Script::Collection(pState, -1, "parameters", server.count())) {
				for (auto& it : server) {
					lua_pushstring(pState, it.first.c_str());
					if (String::ICompare(it.second, "false") == 0 || String::ICompare(it.second, "nil") == 0)
						lua_pushboolean(pState, 0);
					else
						lua_pushlstring(pState, it.second.c_str(), it.second.size());
					lua_rawset(pState, -3); // rawset cause NewIndexProhibited
				}
			}
		} else {
			string value;
			server.getString(name, value);
			if (server.getString(name, value))
				SCRIPT_WRITE_STRING(value.c_str())
		}
	SCRIPT_CALLBACK_RETURN
}

int LUAServer::Set(lua_State* pState) {
	SCRIPT_CALLBACK(ServerConnection,server)
		lua_rawset(pState,1); // consumes key and value
	SCRIPT_CALLBACK_RETURN
}
