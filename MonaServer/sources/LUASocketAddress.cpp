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

#include "LUASocketAddress.h"
#include "LUAIPAddress.h"
#include "ServerConnection.h"

using namespace std;
using namespace Mona;

void LUASocketAddress::Init(lua_State* pState, SocketAddress& address) {
	Script::AddComparator<SocketAddress>(pState);
	lua_getmetatable(pState, -1);
	lua_pushcfunction(pState, &LUASocketAddress::Call);
	lua_setfield(pState, -2, "__call");
	lua_pop(pState, 1);
}

void LUASocketAddress::Clear(lua_State* pState, SocketAddress& address) {
	Script::ClearObject<LUAIPAddress>(pState,address.host());
}

int LUASocketAddress::Call(lua_State* pState) {
	SCRIPT_CALLBACK(SocketAddress,address)
		Script::NewObject<LUASocketAddress>(pState, *new SocketAddress(address));
	SCRIPT_CALLBACK_RETURN
}

int LUASocketAddress::Get(lua_State *pState) {
	SCRIPT_CALLBACK(SocketAddress,address)
		const char* name = SCRIPT_READ_STRING(NULL);
		
		if (name) {
			if(strcmp(name,"host")==0) {
				Script::AddObject<LUAIPAddress>(pState,address.host());
				SCRIPT_FIX_RESULT
			} else if(strcmp(name,"port")==0) {
				SCRIPT_WRITE_NUMBER(address.port())
				SCRIPT_FIX_RESULT
			} else if(strcmp(name,"isIPv6")==0) {
				SCRIPT_WRITE_BOOL(address.family()==IPAddress::IPv6)
				SCRIPT_FIX_RESULT
			} else if (strcmp(name,"value")==0) {
				SCRIPT_WRITE_STRING(address.toString().c_str());
				SCRIPT_FIX_RESULT
			}
		}
	SCRIPT_CALLBACK_RETURN
}

bool LUASocketAddress::Read(Exception& ex, lua_State *pState, int index, SocketAddress& address,bool withDNS) {

	if (lua_isnumber(pState, index)) {
		// just port?
		address.set(IPAddress::Wildcard(), (UInt16)lua_tonumber(pState, index));
		return true;
	}

	if (lua_type(pState, index)==LUA_TSTRING) { // lua_type because can be encapsulated in a lua_next
		const char* value = lua_tostring(pState,index);
		if (lua_type(pState, ++index)==LUA_TSTRING)
			return withDNS ? address.setWithDNS(ex, value, lua_tostring(pState,index)) : address.set(ex, value, lua_tostring(pState,index));
		if (lua_isnumber(pState,index))
			return withDNS ? address.setWithDNS(ex, value, (UInt16)lua_tonumber(pState,index)) : address.set(ex, value, (UInt16)lua_tonumber(pState,index));
		return withDNS ? address.setWithDNS(ex, value) : address.set(ex, value);
	}
	
	if(lua_istable(pState,index)) {
		bool isConst;

		SocketAddress* pOther = Script::ToObject<SocketAddress>(pState, isConst, index);
		if (pOther) {
			address.set(*pOther);
			return true;
		}

		IPAddress* pHost = Script::ToObject<IPAddress>(pState, isConst, index);
		if (pHost) {
			if (lua_type(pState, ++index)==LUA_TSTRING)
				return address.set(ex, *pHost, lua_tostring(pState, index));
			if (lua_isnumber(pState, index)) {
				address.set(*pHost, (UInt16)lua_tonumber(pState, index));
				return true;
			}
			return address.set(ex, *pHost, 0);
		}

		ServerConnection* pServer = Script::ToObject<ServerConnection>(pState, isConst, index);
		if (pServer) {
			address.set(pServer->address);
			return true;
		}

	}

	ex.set(Exception::SOFTWARE, "No valid SocketAddress available to read");
	return false;
}

int LUASocketAddress::Set(lua_State *pState) {
	SCRIPT_CALLBACK(SocketAddress, address)
		lua_rawset(pState,1); // consumes key and value
	SCRIPT_CALLBACK_RETURN
}