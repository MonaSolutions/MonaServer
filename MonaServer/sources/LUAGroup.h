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

#pragma once


#include "Mona/Group.h"
#include "Script.h"

class LUAGroup {
public:

	static void Init(lua_State *pState, Mona::Group& group);
	static void Clear(lua_State* pState, Mona::Group& group) {}

	// -1 must be the client table!
	static void AddClient(lua_State* pState, int indexGroup);
	// -1 must be the group table!
	static void RemoveClient(lua_State* pState, Mona::Client& client);

	static int Get(lua_State *pState);
	static int Set(lua_State *pState);

	static int Item(lua_State *pState);

private:
	static int	Size(lua_State* pState);

};

