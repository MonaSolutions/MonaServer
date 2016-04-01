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

#include "LUAIPAddress.h"
#include "ServerConnection.h"

using namespace std;
using namespace Mona;

void LUAIPAddress::Init(lua_State* pState, IPAddress& address) {
	Script::AddComparator<IPAddress>(pState);
	lua_getmetatable(pState, -1);
	lua_pushcfunction(pState, &LUAIPAddress::Call);
	lua_setfield(pState, -2, "__call");
	lua_pop(pState, 1);
}

int LUAIPAddress::Call(lua_State *pState) {
	SCRIPT_CALLBACK(IPAddress, address)
		Script::NewObject<LUAIPAddress>(pState,*new IPAddress(address));
	SCRIPT_CALLBACK_RETURN
}

int LUAIPAddress::Get(lua_State *pState) {
	SCRIPT_CALLBACK(IPAddress, address)
		const char* name = SCRIPT_READ_STRING(NULL);
		
		if (name) {
			if (strcmp(name, "isWildcard") == 0) {
				SCRIPT_WRITE_BOOL(address.isWildcard())
				SCRIPT_FIX_RESULT
			} else if (strcmp(name, "isBroadcast") == 0) {
				SCRIPT_WRITE_BOOL(address.isBroadcast())
				SCRIPT_FIX_RESULT
			} else if (strcmp(name, "isAnyBroadcast") == 0) {
				SCRIPT_WRITE_BOOL(address.isAnyBroadcast())
				SCRIPT_FIX_RESULT
			} else if (strcmp(name, "isLoopback") == 0) {
				SCRIPT_WRITE_BOOL(address.isLoopback())
				SCRIPT_FIX_RESULT
			} else if (strcmp(name, "isMulticast") == 0) {
				SCRIPT_WRITE_BOOL(address.isMulticast())
				SCRIPT_FIX_RESULT
			} else if (strcmp(name, "isUnicast") == 0) {
				SCRIPT_WRITE_BOOL(address.isUnicast())
				SCRIPT_FIX_RESULT
			} else if (strcmp(name, "isLinkLocal") == 0) {
				SCRIPT_WRITE_BOOL(address.isLinkLocal())
				SCRIPT_FIX_RESULT
			} else if (strcmp(name, "isSiteLocal") == 0) {
				SCRIPT_WRITE_BOOL(address.isSiteLocal())
				SCRIPT_FIX_RESULT
			} else if (strcmp(name, "isIPv4Compatible") == 0) {
				SCRIPT_WRITE_BOOL(address.isIPv4Compatible())
				SCRIPT_FIX_RESULT
			} else if (strcmp(name, "isIPv4Mapped") == 0) {
				SCRIPT_WRITE_BOOL(address.isIPv4Mapped())
				SCRIPT_FIX_RESULT
			} else if (strcmp(name, "isWellKnownMC") == 0) {
				SCRIPT_WRITE_BOOL(address.isWellKnownMC())
				SCRIPT_FIX_RESULT
			} else if (strcmp(name, "isNodeLocalMC") == 0) {
				SCRIPT_WRITE_BOOL(address.isNodeLocalMC())
				SCRIPT_FIX_RESULT
			} else if (strcmp(name, "isLinkLocalMC") == 0) {
				SCRIPT_WRITE_BOOL(address.isLinkLocalMC())
				SCRIPT_FIX_RESULT
			} else if (strcmp(name, "isSiteLocalMC") == 0) {
				SCRIPT_WRITE_BOOL(address.isSiteLocalMC())
				SCRIPT_FIX_RESULT
			} else if (strcmp(name, "isOrgLocalMC") == 0) {
				SCRIPT_WRITE_BOOL(address.isOrgLocalMC())
				SCRIPT_FIX_RESULT
			} else if (strcmp(name, "isGlobalMC") == 0) {
				SCRIPT_WRITE_BOOL(address.isGlobalMC())
				SCRIPT_FIX_RESULT
			} else if (strcmp(name, "isLocal") == 0) {
				SCRIPT_WRITE_BOOL(address.isLocal())
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


bool LUAIPAddress::Read(Exception& ex, lua_State *pState, int index, IPAddress& address,bool withDNS) {

	if (lua_type(pState, index)==LUA_TSTRING) // lua_type because can be encapsulated in a lua_next
		return withDNS ? address.setWithDNS(ex, lua_tostring(pState,index)) : address.set(ex, lua_tostring(pState,index));

	if(lua_istable(pState,index)) {
		bool isConst;

		IPAddress* pOther = Script::ToObject<IPAddress>(pState, isConst, index);
		if (pOther) {
			address.set(*pOther);
			return true;
		}

		SocketAddress* pAddress = Script::ToObject<SocketAddress>(pState, isConst, index);
		if (pAddress) {
			address.set(pAddress->host());
			return true;
		}

		ServerConnection* pServer = Script::ToObject<ServerConnection>(pState, isConst, index);
		if (pServer) {
			address.set(pServer->address.host());
			return true;
		}
	}

	ex.set(Exception::NETIP, "No valid IPAddress available to read");
	return false;
}

int LUAIPAddress::Set(lua_State *pState) {
	SCRIPT_CALLBACK(IPAddress, address)
		lua_rawset(pState,1); // consumes key and value
	SCRIPT_CALLBACK_RETURN
}