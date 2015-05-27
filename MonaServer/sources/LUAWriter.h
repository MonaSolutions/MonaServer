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

#include "Mona/Writer.h"
#include "Script.h"

class LUAWriter {
public:
	LUAWriter(lua_State* pState,Mona::Writer& parentWriter);
	Mona::Writer&		writer;

	static void Init(lua_State *pState, Mona::Writer& writer) {}
	static void Clear(lua_State* pState, Mona::Writer& writer);
	static void Delete(lua_State* pState, Mona::Writer& writer) { writer.close(); }

	static int Get(lua_State *pState);
	static int Set(lua_State *pState);

private:
	static int	Flush(lua_State *pState);
	static int	WriteMessage(lua_State *pState);
	static int	WriteInvocation(lua_State *pState);
	static int  WriteRaw(lua_State* pState);
	static int	NewWriter(lua_State *pState);
	static int	Close(lua_State *pState);
	

	Mona::Writer::OnClose::Type onClose;

	lua_State*	_pState;
};


