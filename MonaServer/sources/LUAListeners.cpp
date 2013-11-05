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

#include "LUAListeners.h"
#include "LUAListener.h"
#include "LUAClient.h"
#include "Mona/Listeners.h"


using namespace std;
using namespace Mona;

const char*		LUAListeners::Name="Mona::Listeners";

int LUAListeners::Pairs(lua_State* pState) {
	SCRIPT_CALLBACK(Listeners,LUAListeners,listeners)
		lua_getglobal(pState,"next");
		if(!lua_iscfunction(pState,-1))
			SCRIPT_ERROR("'next' should be a LUA function, it should not be overloaded")
		else {
			lua_newtable(pState);
			for (auto it : listeners) {
				SCRIPT_WRITE_PERSISTENT_OBJECT(Listener,LUAListener,*it.second)
				SCRIPT_WRITE_PERSISTENT_OBJECT(Client,LUAClient,*it.first)
				lua_rawset(pState,-3);
			}
		}
	SCRIPT_CALLBACK_RETURN
}

int LUAListeners::Get(lua_State *pState) {
	SCRIPT_CALLBACK(Listeners,LUAListeners,listeners)
		string name = SCRIPT_READ_STRING("");
		if(name=="pairs")
			SCRIPT_WRITE_FUNCTION(&LUAListeners::Pairs)
		else if(name == "count")
			SCRIPT_WRITE_NUMBER(listeners.count())
	SCRIPT_CALLBACK_RETURN
}


int LUAListeners::Set(lua_State *pState) {
	SCRIPT_CALLBACK(Listeners,LUAListeners,listeners)
		string name = SCRIPT_READ_STRING("");
		lua_rawset(pState,1); // consumes key and value
	SCRIPT_CALLBACK_RETURN
}


