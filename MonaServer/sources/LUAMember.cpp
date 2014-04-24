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
#include "LUAClient.h"
#include "Mona/Util.h"

using namespace Mona;
using namespace std;


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
				LUAClient::GetID(pState,member);
			} else if (strcmp(name, "rawId") == 0)
				SCRIPT_WRITE_BINARY(member.id,ID_SIZE)
			else if (strcmp(name, "release") == 0)
				SCRIPT_WRITE_FUNCTION(&LUAMember::Release);
		}
	SCRIPT_CALLBACK_RETURN
}

int LUAMember::Set(lua_State *pState) {
	SCRIPT_CALLBACK(Peer,member)
		lua_rawset(pState,1); // consumes key and value
	SCRIPT_CALLBACK_RETURN
}
