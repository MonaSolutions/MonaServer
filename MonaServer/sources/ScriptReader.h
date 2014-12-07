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

#include "Mona/ReferableReader.h"
#include "Script.h"


class ScriptReader : public Mona::ReferableReader {
public:
	// Read the count number of lua object in the lua_State stack
	ScriptReader(lua_State *pState, Mona::UInt32 count);

#if defined(_DEBUG)
	Mona::UInt32	read(Mona::DataWriter& writer,Mona::UInt32 count=END);
#endif

	void reset();

private:
	Mona::UInt8			followingType();
	bool				readOne(Mona::UInt8 type, Mona::DataWriter& writer);

	bool				writeNext(Mona::DataWriter& writer);

	lua_State*			_pState;			
	int					_start;
	int					_current;
	int					_end;
};
