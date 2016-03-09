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

#include "ScriptWriter.h"


using namespace std;
using namespace Mona;


ScriptWriter::ScriptWriter(lua_State *pState) : _top(lua_gettop(pState)), _pState(pState) {
	
}

ScriptWriter::~ScriptWriter() {
	// clear references
	for (int reference : _references)
		luaL_unref(_pState, LUA_REGISTRYINDEX, reference);
}

void ScriptWriter::clear() {
	lua_pop(_pState, lua_gettop(_pState) - _top);
	_layers.clear();
	// clear references
	for (int reference : _references)
		luaL_unref(_pState, LUA_REGISTRYINDEX, reference);
	_references.clear();
}

bool ScriptWriter::repeat(UInt64 reference) {
	bool pushed(start());
	lua_rawgeti(_pState, LUA_REGISTRYINDEX,(int)reference);
	if (lua_isnil(_pState, -1)) {
		lua_pop(_pState, pushed ? 2 : 1);
		return false;
	}
	end();
	return true;
}

UInt64 ScriptWriter::writeDate(const Date& date) {
	start();

	lua_newtable(_pState);
	lua_pushnumber(_pState, (double)date);
	lua_setfield(_pState, -2, "__time");

	// Attribut of LUA date are exprimed in LOCAL (there is no offset informations)
	Date dateLocal(date,Date::LOCAL);

	lua_pushnumber(_pState, dateLocal.year());
	lua_setfield(_pState, -2, "year");
	lua_pushnumber(_pState, dateLocal.month() + 1);
	lua_setfield(_pState, -2, "month");
	lua_pushnumber(_pState, dateLocal.day());
	lua_setfield(_pState, -2, "day");
	lua_pushnumber(_pState, dateLocal.yearDay());
	lua_setfield(_pState, -2, "yday");
	lua_pushnumber(_pState, dateLocal.weekDay());
	lua_setfield(_pState, -2, "wday");
	lua_pushnumber(_pState, dateLocal.hour());
	lua_setfield(_pState, -2, "hour");
	lua_pushnumber(_pState, dateLocal.minute());
	lua_setfield(_pState, -2, "min");
	lua_pushnumber(_pState, dateLocal.second());
	lua_setfield(_pState, -2, "sec");
	lua_pushnumber(_pState, dateLocal.millisecond());
	lua_setfield(_pState, -2, "msec");
	lua_pushboolean(_pState, dateLocal.isDST() ? 1 : 0);
	lua_setfield(_pState, -2, "isdst");

	UInt64 ref(reference());
	end();
	return ref;
}

UInt64 ScriptWriter::writeBytes(const UInt8* data,UInt32 size) {
	start();
	lua_newtable(_pState);
	lua_pushlstring(_pState,(const char*)data,size);
	lua_setfield(_pState,-2,"__raw");
	UInt64 ref(reference());
	end();
	return ref;
}


UInt64 ScriptWriter::beginObject(const char* type) {
	start();
	lua_newtable(_pState);
	if(type) {
		lua_pushstring(_pState,type);
		lua_setfield(_pState,-2,"__type");
	}
	_layers.push_back(0);
	return reference();
}

UInt64 ScriptWriter::beginArray(UInt32 size) {
	start();
	lua_newtable(_pState);
	_layers.push_back(1);
	return reference();
}

UInt64 ScriptWriter::beginObjectArray(UInt32 size) {
	start();
	lua_newtable(_pState);
	_layers.push_back(-1);
	return reference();
}

UInt64 ScriptWriter::beginMap(Exception& ex, UInt32 size, bool weakKeys) {
	start();
	lua_newtable(_pState);
	lua_pushnumber(_pState,size);
	lua_setfield(_pState,-2,"__size");
	if(weakKeys) {
		lua_newtable(_pState);
		lua_pushliteral(_pState,"k");
		lua_setfield(_pState,-2,"__mode");
		lua_setmetatable(_pState,-2);
	}
	_layers.push_back(-3);
	return reference();
}

UInt64 ScriptWriter::reference() {
	// record last reference
	lua_pushvalue(_pState, -1);
	int reference(luaL_ref(_pState, LUA_REGISTRYINDEX));
	_references.emplace_back(reference);
	return (UInt64)reference;
}

bool ScriptWriter::start() {
	if (_layers.empty())
		return false;
	int& type = _layers.back();
	if(type <= 0)
		return false;
	lua_pushnumber(_pState, type++);
	return true;
}

void ScriptWriter::end() {
	if (_layers.empty())
		return;
	int& type = _layers.back();
	if (type >= -1) // mixed, object or array
		lua_rawset(_pState, -3);
	else if (type == -2) {  // map value
		lua_rawset(_pState, -3);
		--type;
	} else // map key
		++type;
}

void ScriptWriter::endComplex() {
	if(_layers.empty()) {
		SCRIPT_BEGIN(_pState)
			SCRIPT_ERROR("end struct called without begin struct calling");
		SCRIPT_END
		return;
	}

	/* Allow to create method on typed object!
	function onTypedObject(type,object)
	  if type=="Cat" then
		function object:meow()
		  print("meow")
		end
	  end
	end*/
	lua_getfield(_pState, -1, "__type");
	if (lua_isstring(_pState, -1)) {
		int top=lua_gettop(_pState);
		SCRIPT_BEGIN(_pState)
			SCRIPT_FUNCTION_BEGIN("onTypedObject", LUA_ENVIRONINDEX)
				lua_pushvalue(_pState, top); // type
				lua_pushvalue(_pState, top-1); // object
				SCRIPT_FUNCTION_CALL
			SCRIPT_FUNCTION_END
		SCRIPT_END
	}
	lua_pop(_pState, 1);

	if (_layers.back()!=-1) {
		_layers.pop_back();
		return end();
	}
	_layers.back()=1; // mixed object, now we are writing the array part
}
