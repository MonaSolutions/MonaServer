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

#include "LUAPersistentTable.h"
#include "Mona/FileSystem.h"

using namespace std;
using namespace Mona;

void LUAPersistentTable::Init(lua_State* pState, LUAPersistentTable& table) {
	lua_getmetatable(pState, -1);
	lua_pushcfunction(pState,&LUAPersistentTable::Len);
	lua_setfield(pState, -2, "__len");
	lua_newtable(pState);
	lua_setfield(pState,-2, "|items");
	lua_pop(pState, 1);
}

int	LUAPersistentTable::Len(lua_State* pState) {
	SCRIPT_CALLBACK(LUAPersistentTable,table)
		SCRIPT_WRITE_NUMBER(table.count)
	SCRIPT_CALLBACK_RETURN
}

int LUAPersistentTable::Get(lua_State *pState) {
	SCRIPT_CALLBACK(LUAPersistentTable, table)
		const char* name = SCRIPT_READ_STRING(NULL);
		if (name) {
			lua_getmetatable(pState, 1);
			lua_getfield(pState, -1, "|items");
			lua_replace(pState, -2);
			lua_getfield(pState, -1, name);
			lua_replace(pState, -2);
			if (lua_isnil(pState,-1) && strcmp(name, "count") == 0)
				SCRIPT_WRITE_NUMBER(table.count)
		}
	SCRIPT_CALLBACK_RETURN
}

int LUAPersistentTable::Set(lua_State *pState) {
	SCRIPT_CALLBACK(LUAPersistentTable, table);
		const char* name = SCRIPT_READ_STRING(NULL);
		if (name) {
			Int8 removing = SCRIPT_NEXT_TYPE==LUA_TNIL ? 1 : 0;
			string path;
			FileSystem::MakeFolder(String::Format(path, table.path, '/', name));

			lua_getmetatable(pState, 1);
			lua_getfield(pState, -1, "|items");
			lua_getfield(pState, -1, name); // old value

			if (!removing) {
				if (lua_istable(pState, -1)) {
					SCRIPT_ERROR("Complex type exists already on ", path, ", for secure reasons delete explicitly this entry with a nil assignation before override")
					removing = -1;
				}
			}
				
			if (removing>=0) {
				if (SCRIPT_NEXT_TYPE==LUA_TTABLE) {

					lua_pushvalue(pState, 2); // name
					Script::NewObject<LUAPersistentTable,LUAPersistentTable>(pState,*new LUAPersistentTable(table.persistentData, path));

					// table iteration
					lua_pushnil(pState);  // first key 
					while (Script::Next(pState, 3) != 0) {
						// uses 'key' (at index -2) and 'value' (at index -1) 
						lua_pushvalue(pState, -2); // duplicate key
						lua_pushvalue(pState, -2); // duplicate value
						lua_settable(pState, -5);
						lua_pop(pState, 1);
					}

					// memory
					lua_rawset(pState, -4);
					
				} else {
					// Primitive type!
					Exception ex;
					bool success = false;
					if (removing) {
						if ((success = table.persistentData.remove(ex, path)) && !lua_isnil(pState, -1) && !lua_istable(pState, -1))
							--table.count;
					} else {
						UInt32 size(0);
						const UInt8* value = Serialize(pState,3,size);
						if((success = table.persistentData.add(ex, path, value , size)) && lua_isnil(pState, -1))
							++table.count;
					}
					if (success) {
						// memory
						lua_pushvalue(pState, 2); // name
						lua_pushvalue(pState, 3); // value
						lua_rawset(pState, -4);
						if (ex)
							SCRIPT_WARN("Database entry ", path, " writing, ", ex.error())
					} else
						SCRIPT_ERROR("Database entry ", path, " writing, ", ex.error())
				}
			}
			
			lua_pop(pState, 3);
			
		} else
			SCRIPT_ERROR("Key database entry must be a string or a number");
	SCRIPT_CALLBACK_RETURN;
}

const UInt8* LUAPersistentTable::Serialize(lua_State* pState, int index, UInt32& size) {
	// value to stringize is in the top
	switch (lua_type(pState, index)) {
		case LUA_TLIGHTUSERDATA:
		case LUA_TFUNCTION:
		case LUA_TUSERDATA:
		case LUA_TTHREAD:
		case LUA_TTABLE: {
			string pointer;
			String::Format(pointer, lua_topointer(pState, index));
			size = pointer.size();
			return (const UInt8*)pointer.c_str();
		}
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
			return (const UInt8*)"null";
		default:
			size = lua_objlen(pState, index);
			return (const UInt8*)lua_tostring(pState, index);
	}
}
