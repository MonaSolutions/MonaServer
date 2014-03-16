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

#include "LUADataTable.h"

using namespace std;
using namespace Mona;

void LUADataTable::Init(lua_State* pState, LUADataTable& table) {
	lua_getmetatable(pState, -1);

	lua_pushnumber(pState, 0);
	lua_setfield(pState, -2, "|count");

	lua_newtable(pState);
	// metatable
	lua_newtable(pState);
	lua_pushlightuserdata(pState, (void*)&table);
	lua_setfield(pState, -2, "|table");

	lua_pushcfunction(pState, &LUADataTable::NewIndex);
	lua_setfield(pState, -2, "__newindex");

#if !defined(_DEBUG)
	lua_pushstring(pState, "change metatable of datatable values is prohibited");
	lua_setfield(pState, -2, "__metatable");
#endif

	lua_setmetatable(pState, -2);
	lua_setfield(pState, -2, "|items");
	lua_pop(pState, 1);
}

int	LUADataTable::Destroy(lua_State* pState) {
	SCRIPT_DESTRUCTOR_CALLBACK(LUADataTable, table)
		delete &table;
	SCRIPT_CALLBACK_RETURN
}

int LUADataTable::Get(lua_State *pState) {
	SCRIPT_CALLBACK(LUADataTable, table)
	if (SCRIPT_NEXT_TYPE == LUA_TSTRING || SCRIPT_NEXT_TYPE == LUA_TNUMBER) {
		lua_getmetatable(pState, 1);
		lua_getfield(pState, -1, "|items");
		lua_replace(pState, -2);
		if (!lua_isnil(pState, -1)) {
			lua_getfield(pState, -1, lua_tostring(pState, 2));
			lua_replace(pState, -2);
		}
	} else
		SCRIPT_ERROR("Key database entry must always be a string or a number"); // return nil
	SCRIPT_CALLBACK_RETURN
}

int LUADataTable::NewIndex(lua_State *pState) {
	// 1 => table |items
	// 2 => key
	// 3 => value
	if (lua_getmetatable(pState, 1)) {
		lua_getfield(pState,-1, "|table");
		LUADataTable* pTable = (LUADataTable*)lua_touserdata(pState, -1);
		if (pTable)
			pTable->newInsertion = true;
		lua_pop(pState, 2);
	}
	lua_rawset(pState, 1);
	return 0;
}

int LUADataTable::Set(lua_State *pState) {
	SCRIPT_CALLBACK(LUADataTable, table);
		if (SCRIPT_NEXT_TYPE == LUA_TSTRING || SCRIPT_NEXT_TYPE == LUA_TNUMBER) {
			string name(lua_tostring(pState, 2));

			bool removing = lua_isnil(pState, 3) == 1;
			bool prohibited = !removing && !table.complexData.empty() && table.complexData.find(name)!=table.complexData.end();
			string path;
			String::Format(path, table.path, '/', name);
			if (prohibited)
				SCRIPT_ERROR("Complex type exists already on ", path, ", for secure reasons delete explicitly this entry before with a nil assignation")
			else {
				// load values table
				lua_getmetatable(pState, 1);
				lua_getfield(pState, -1, "|items");
		
				if (lua_istable(pState, 3)) {
					SCRIPT_NEW_OBJECT(LUADataTable, LUADataTable, new LUADataTable(table.database, path));
					// table iteration
					lua_pushnil(pState);  // first key 
					while (lua_next(pState, 3) != 0) {
						// uses 'key' (at index -2) and 'value' (at index -1) 
						lua_pushvalue(pState, -2); // duplicate key
						lua_pushvalue(pState, -2); // duplicate value
						lua_settable(pState, -5);
						lua_pop(pState, 1);
					}
					// memory
					table.newInsertion = false;
					lua_setfield(pState, -2, name.c_str());
					if (table.newInsertion) {
						lua_pushnumber(pState, ++table.count);
						lua_setfield(pState, -3, "|count");
						table.complexData.insert(name);
					}
				} else {
					// Primitive type!
					Exception ex;
					bool success = false;
					if (removing) {
						success = table.database.remove(ex, path);
					} else {
						UInt32 size(0);
						const UInt8* value = table.serialize(pState, 3, size);
						success = table.database.add(ex, path, value, size);
					}
					if (success) {
						// memory
						lua_pushvalue(pState, 3);
						table.newInsertion = false;
						lua_setfield(pState, -2, name.c_str());
						if (table.newInsertion) {
							if (!removing) {
								lua_pushnumber(pState, ++table.count);
								lua_setfield(pState, -3, "|count");
							}
						} else if (removing) {
							lua_pushnumber(pState, --table.count);
							lua_setfield(pState, -3, "|count");
							table.complexData.erase(name);
						}
					} else if (ex)
						SCRIPT_ERROR("Database entry ", path, " writing failed(", ex.error(), ")")
					else
						SCRIPT_ERROR("Database entry ", path, "writing failed (Unknown error)");
				}

				lua_pop(pState, 2); // remove |items table and metatable of LUADataTable
			}

		} else
			SCRIPT_ERROR("Key database entry must always be a string");
	SCRIPT_CALLBACK_RETURN;
}

const UInt8* LUADataTable::serialize(lua_State* pState, int index, UInt32& size) {
	// value to stringize is in the top
	switch (lua_type(pState, index)) {
		case LUA_TLIGHTUSERDATA:
		case LUA_TFUNCTION:
		case LUA_TUSERDATA:
		case LUA_TTHREAD:
		case LUA_TTABLE:
			String::Format(_buffer, lua_topointer(pState, index));
			size = _buffer.size();
			return (const UInt8*)_buffer.c_str();
		case LUA_TBOOLEAN: {
			bool value = lua_toboolean(pState, index) ? true : false;
			if (value) {
				size = 4;
				return (const UInt8*)"true";
			}
			size = 5;
			return (const UInt8*)"false";
		}
		case LUA_TNIL:
			size = 3;
			return (const UInt8*)"nil";
		default:
			size = lua_objlen(pState, index);
			return (const UInt8*)lua_tostring(pState, index);
	}
}
