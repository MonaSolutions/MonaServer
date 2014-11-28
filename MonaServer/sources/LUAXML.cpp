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

#include "LUAXML.h"

using namespace std;
using namespace Mona;


//////////////   XML TO LUA   /////////////////////

bool LUAXML::LUAToXML(Exception& ex, lua_State* pState, int index, const PoolBuffers& poolBuffers) {
	// index - LUA XML table
	if (!lua_istable(pState, index)) {
		ex.set(Exception::APPLICATION, "Just LUA table can be convert to XML");
		return false;
	}

	PacketWriter writer(poolBuffers);
	const char* name(NULL);
	bool result(false);
	
	lua_pushnil(pState);  // first key 
	while (lua_next(pState, index) != 0) {
		// uses 'key' (at index -2) and 'value' (at index -1) 
		if (lua_type(pState, -2) == LUA_TSTRING) {

			name = lua_tostring(pState, -2);

			if (lua_istable(pState, -1)) {

				// information fields { xml = {version = 1.0}, ... }

				writer.write("<?").write(name).write(" ");

				lua_pushnil(pState);  // first key 
				while (lua_next(pState, -2) != 0) {
					// uses 'key' (at index -2) and 'value' (at index -1) 
					if (lua_type(pState, -2) == LUA_TSTRING && lua_isstring(pState, -1))
						writer.write(lua_tostring(pState, -2)).write("=\"").write(lua_tostring(pState, -1)).write("\" ");
					else
						ex.set(Exception::APPLICATION, "Bad ", name, " XML attribute information, key must be string and value convertible to string");
					lua_pop(pState, 1);
				}

				writer.write("?>");
			} else
				ex.set(Exception::APPLICATION, "Bad ",name," XML information, value must be a table");

		} else if (lua_isnumber(pState, -2)) {
			if(lua_istable(pState, -1))
				result = LUAToXMLElement(ex, pState, writer);
			else
				ex.set(Exception::APPLICATION, "Impossible to write inner XML data on the top of XML level");
		}  else
			ex.set(Exception::APPLICATION, "Impossible to convert the key of type ",lua_typename(pState,lua_type(pState,-2)),"to a correct XML root element");

		lua_pop(pState, 1);
	}

	if (result)
		lua_pushlstring(pState,STR writer.data(), writer.size());

	return result;
}


bool LUAXML::LUAToXMLElement(Exception& ex, lua_State* pState, PacketWriter& writer) {
	// -1 => table

	lua_pushstring(pState, "__name");
	lua_rawget(pState, -2);
	const char* name(lua_tostring(pState, -1));
	lua_pop(pState, 1);
	if (!name) {
		ex.set(Exception::APPLICATION, "Impossible to write a XML element without name (__name)");
		return false;
	}

	// write attributes
	writer.write("<").write(name).write(" ");
	lua_pushnil(pState);  // first key 
	while (lua_next(pState, -2) != 0) {
		// uses 'key' (at index -2) and 'value' (at index -1) 
		if (lua_type(pState, -2) == LUA_TSTRING && strcmp(lua_tostring(pState, -2),"__name")!=0 && lua_isstring(pState, -1))
			writer.write(lua_tostring(pState, -2)).write("=\"").write(lua_tostring(pState, -1)).write("\" ");
		lua_pop(pState, 1);
	}
	if (lua_objlen(pState, -1) == 0) {
		writer.write("/>");
		return true;
	}

	writer.write(">");

	// write elements
	lua_pushnil(pState);  // first key 
	while (lua_next(pState, -2) != 0) {
		// uses 'key' (at index -2) and 'value' (at index -1) 
		if (lua_isnumber(pState, -2)) {
			if (lua_istable(pState, -1))
				LUAToXMLElement(ex, pState, writer); // table child
			else if (lua_isstring(pState, -1))
				writer.write(lua_tostring(pState, -1), lua_objlen(pState, -1)); // __value
			else
				ex.set(Exception::APPLICATION, "Impossible to convert the value of type ", lua_typename(pState, lua_type(pState, -1)), "to a correct XML value of ",name);
		} else if (lua_type(pState, -2) != LUA_TSTRING)
			ex.set(Exception::APPLICATION, "Impossible to convert the key of type ",lua_typename(pState,lua_type(pState,-2)),"to a correct XML element of ",name);
		lua_pop(pState, 1);
	}

	writer.write("</").write(name).write(">");
	return true;
}


//////////////   LUA TO XML   /////////////////////


int LUAXML::Index(lua_State* pState) {
	// 1 - table
	// 2 - key not found

	const char* key(lua_tostring(pState,2));
	if (!key)
		return 0;

	if (strcmp(key, "__value") == 0) {
		// return the first index value
		lua_rawgeti(pState, 1, 1);
		return 1;
	}

	lua_pushnil(pState);  // first key 
	while (lua_next(pState, 1) != 0) {
		// uses 'key' (at index -2) and 'value' (at index -1) 
		if (lua_type(pState, -2) == LUA_TNUMBER && lua_type(pState, -1) == LUA_TTABLE) {
			lua_pushstring(pState, "__name");
			lua_rawget(pState, -2);
			const char* name(lua_tostring(pState, -1));
			lua_pop(pState, 1); // remove __name
			if (name && strcmp(key, name) == 0) {
				lua_replace(pState, -2);
				return 1;
			}
		}
		lua_pop(pState, 1);
	}
	
	return 0; // not found!
}

int LUAXML::NewIndex(lua_State* pState) {
	// 1 - table
	// 2 - key not found
	// 3 - value
	const char* key(lua_tostring(pState,2));
	if (key && strcmp(key, "__value") == 0) {
		lua_pushvalue(pState, 3);
		lua_rawseti(pState, 1, 1);
	}
	return 0;
}

bool LUAXML::onXMLInfos(const char* name, Parameters& attributes) {
	lua_newtable(_pState);
	Parameters::ForEach forEach([this](const string& key, const string& value) {
		lua_pushstring(_pState,value.c_str());
		lua_setfield(_pState, -2, key.c_str());
	});
	attributes.iterate(forEach);
	lua_setfield(_pState, _firstIndex, name);
	return true;
}

bool LUAXML::onStartXMLElement(const char* name, Parameters& attributes) {
	lua_newtable(_pState);
	lua_pushstring(_pState, name);
	lua_setfield(_pState, -2, "__name");

	Parameters::ForEach forEach([this](const string& key, const string& value) {
		lua_pushstring(_pState,value.c_str());
		lua_setfield(_pState, -2, key.c_str());
	});
	attributes.iterate(forEach);

	return true;
}

bool LUAXML::onInnerXMLElement(const char* name, const char* data, UInt32 size) {
	lua_pushlstring(_pState,data,size);
	lua_rawseti(_pState,-2,lua_objlen(_pState,-2)+1);
	return true;
}

bool LUAXML::onEndXMLElement(const char* name) {
	addMetatable();
	lua_rawseti(_pState,-2,lua_objlen(_pState,-2)+1); // set in parent table
	return true;
}

void LUAXML::addMetatable() {
	// metatable
	lua_newtable(_pState);
#if !defined(_DEBUG)
	lua_pushliteral(_pState, "change metatable of XML DOM object is prohibited");
	lua_setfield(_pState, -2, "__metatable");
#endif
	lua_pushcfunction(_pState, &LUAXML::Index);
	lua_setfield(_pState, -2, "__index");
	lua_pushcfunction(_pState, &LUAXML::NewIndex);
	lua_setfield(_pState, -2, "__newindex");
	lua_setmetatable(_pState, -2);
}


