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


#include "Mona/DataWriter.h"
#include "Script.h"
#include <vector>


class ScriptWriter : public Mona::DataWriter{
public:
	ScriptWriter(lua_State *pState);
	virtual ~ScriptWriter();

	Mona::UInt64 beginObject(const char* type=NULL);
	void		 writePropertyName(const char* name) { lua_pushstring(_pState, name); }
	void		 endObject() { endComplex(); }

	Mona::UInt64 beginArray(Mona::UInt32 size);
	void		 endArray() { endComplex(); }

	Mona::UInt64 beginObjectArray(Mona::UInt32 size);

	void		 writeNumber(double value) { start();  lua_pushnumber(_pState, value); end(); }
	void		 writeString(const char* value,Mona::UInt32 size) { start(); lua_pushlstring(_pState,value,size); end(); }
	void		 writeBoolean(bool value) { start(); lua_pushboolean(_pState, value); end(); }
	void		 writeNull() { start(); lua_pushnil(_pState); end(); }
	Mona::UInt64 writeDate(const Mona::Date& date);
	Mona::UInt64 writeBytes(const Mona::UInt8* data,Mona::UInt32 size);

	Mona::UInt64 beginMap(Mona::Exception& ex, Mona::UInt32 size, bool weakKeys = false);
	void		 endMap() { endComplex(); }

	void clear();

	bool repeat(Mona::UInt64 reference);

private:
	Mona::UInt64 reference();
	bool start();
	void end();
	void endComplex();

	int			_top;
	lua_State*	_pState;
	
	std::vector<int>	_layers; // int>0 means array, int==0 means object, int==-1 means mixed, int=-3/-2 means map
	std::vector<int>	_references;
};

