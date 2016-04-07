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

#include "Script.h"
#include "Mona/Logs.h"
#include "Mona/Util.h"
#include <math.h>
extern "C" {
	#include "luajit-2.0/lualib.h"
}

using namespace std;
using namespace Mona;


const char* Script::LastError(lua_State *pState) {
	int top = lua_gettop(pState);
	if (top == 0)
		return "Unknown error";
	const char* error = lua_tostring(pState, -1);
	lua_pop(pState, 1);
	if (!error)
		return "Unknown error";
	return error;
}

int Script::Error(lua_State *pState) {
	SCRIPT_BEGIN(pState)
		SCRIPT_LOG(Mona::Logger::LEVEL_ERROR,__FILE__,__LINE__, false, ToPrint(pState,LOG_BUFFER))
	SCRIPT_END
	return 0;
}
int Script::Warn(lua_State *pState) {
	SCRIPT_BEGIN(pState)
		SCRIPT_LOG(Mona::Logger::LEVEL_WARN,__FILE__,__LINE__, false, ToPrint(pState,LOG_BUFFER))
	SCRIPT_END
	return 0;
}
int Script::Note(lua_State *pState) {
	SCRIPT_BEGIN(pState)
		SCRIPT_LOG(Mona::Logger::LEVEL_NOTE,__FILE__,__LINE__, false, ToPrint(pState,LOG_BUFFER))
	SCRIPT_END
	return 0;
}
int Script::Info(lua_State *pState) {
	SCRIPT_BEGIN(pState)
		SCRIPT_LOG(Mona::Logger::LEVEL_INFO,__FILE__,__LINE__, false, ToPrint(pState,LOG_BUFFER))
	SCRIPT_END
	return 0;
}
int Script::Debug(lua_State *pState) {
	SCRIPT_BEGIN(pState)
		SCRIPT_LOG(Mona::Logger::LEVEL_DEBUG,__FILE__,__LINE__, false, ToPrint(pState,LOG_BUFFER))
	SCRIPT_END
	return 0;
}
int Script::Trace(lua_State *pState) {
	SCRIPT_BEGIN(pState)
		SCRIPT_LOG(Mona::Logger::LEVEL_TRACE,__FILE__,__LINE__, false, ToPrint(pState,LOG_BUFFER))
	SCRIPT_END
	return 0;
}

int Script::Panic(lua_State *pState) {
	SCRIPT_BEGIN(pState)
		SCRIPT_FATAL(ToPrint(pState,LOG_BUFFER));
	SCRIPT_END
	return 0;
}

lua_State* Script::CreateState() {
	lua_State* pState = luaL_newstate();

	luaL_openlibs(pState);

	lua_atpanic(pState,&Script::Panic);

	lua_pushcfunction(pState,&Script::Error);
	lua_setglobal(pState,"ERROR");
	lua_pushcfunction(pState,&Script::Warn);
	lua_setglobal(pState,"WARN");
	lua_pushcfunction(pState,&Script::Note);
	lua_setglobal(pState,"NOTE");
	lua_pushcfunction(pState,&Script::Info);
	lua_setglobal(pState,"INFO");
	lua_pushcfunction(pState,&Script::Debug);
	lua_setglobal(pState,"DEBUG");
	lua_pushcfunction(pState,&Script::Trace);
	lua_setglobal(pState,"TRACE");
	lua_pushcfunction(pState, &Script::Pairs);
	lua_setglobal(pState, "pairs");
	lua_pushcfunction(pState, &Script::IPairs);
	lua_setglobal(pState, "ipairs");
	lua_pushcfunction(pState, &Script::Next);
	lua_setglobal(pState, "next");

	// set global metatable
	lua_newtable(pState);
#if !defined(_DEBUG)
	lua_pushliteral(pState, "change metatable of global environment is prohibited");
	lua_setfield(pState, -2, "__metatable");
#endif
	lua_setmetatable(pState, LUA_GLOBALSINDEX);

	return pState;
}

void Script::CloseState(lua_State* pState) {
	if(pState)
		lua_close(pState);
}

void Script::PushValue(lua_State* pState,const char* value) {
	if (String::ICompare(value, "false") == 0)
		lua_pushboolean(pState, 0);
	else if (String::ICompare(value, "true") == 0)
		lua_pushboolean(pState, 1);
	else if (String::ICompare(value, "null") == 0)
		lua_pushnil(pState);
	else {
		double result(0);
		if (String::ToNumber(value, result))
			lua_pushnumber(pState, result);
		else
			lua_pushstring(pState, value);
	}
}

void Script::PushValue(lua_State* pState,const UInt8* value, UInt32 size) {
	if (size==5 && String::ICompare(STR value, EXPAND("false")) == 0)
		lua_pushboolean(pState, 0);
	else if (size==4 && String::ICompare(STR value, EXPAND("true")) == 0)
		lua_pushboolean(pState, 1);
	else if (size==4 && String::ICompare(STR value, EXPAND("null")) == 0)
		lua_pushnil(pState);
	else {
		double result(0);
		if (String::ToNumber(STR value,size, result))
			lua_pushnumber(pState, result);
		else
			lua_pushlstring(pState, STR value,size);
	}
}

 bool Script::GetCollection(lua_State* pState, int index,const char* field) {
	if (!field || !lua_getmetatable(pState, index))
		return false;
	// get collection table
	lua_getfield(pState, -1, field);
	lua_replace(pState, -2); // remove metatable
	if (!lua_istable(pState, -1)) {
		lua_pop(pState, 1);
		return false;
	}
	return true;
}

 
void Script::DeleteCollection(lua_State* pState,int index,const char* field) {
	if (!field || !lua_getmetatable(pState, index))
		return;
	lua_pushnil(pState);
	lua_setfield(pState, -2, field);
	lua_pop(pState, 1); // metatable
}

void Script::FillCollection(lua_State* pState, UInt32 size) {
	int index = -1-(size<<1); // index collection (-1-2*size)

	if (!lua_getmetatable(pState, index)) {
		SCRIPT_BEGIN(pState)
			SCRIPT_ERROR("Invalid collection to fill, no metatable")
		SCRIPT_END
		return;
	}

	lua_getfield(pState, -1, "|items");
	if (!lua_istable(pState, -1)) {
		lua_pop(pState, 2);
		SCRIPT_BEGIN(pState)
			SCRIPT_ERROR("Invalid collection to fill, no |items field in metatable")
		SCRIPT_END
		return;
	}

	lua_insert(pState, index-1); // insert |item to the collection index, just before keys/values
	lua_insert(pState, index-1); // insert metatable just before |items

	int count(0);
	while (size-- > 0) {
		
		if (lua_isstring(pState, -2)) { // key
			const char* key(lua_tostring(pState, -2));
			const char* sub(strchr(key, '.'));
			if (sub) {
				*(char*)sub = '\0';
				Collection(pState, index-2, key);
				*(char*)sub = '.';
				lua_pushstring(pState, sub + 1);
				lua_pushvalue(pState, -3); // value
				FillCollection(pState, 1);
				lua_pop(pState, 1);
			}
		}

		// check if key exists already to update count
		lua_pushvalue(pState, -2); // key
		lua_rawget(pState, index-1);
		if (!lua_isnil(pState, -1)) { // if old value exists
			if (lua_isnil(pState, -2)) // if new value is nil (erasing)
				--count;
		} else if (lua_isnil(pState, -1))  // if old value doesn't exists
			++count;
		lua_pop(pState, 1);

		lua_rawset(pState,index);
		index += 2;
	}

	lua_pop(pState, 1); // remove |items

	lua_getfield(pState,-1,"|count");
	lua_pushnumber(pState,lua_tonumber(pState,-1)+count);
	lua_replace(pState, -2);
	lua_setfield(pState,-2,"|count");
	lua_pop(pState,1); // remove metatable

}

void Script::ClearCollection(lua_State* pState) {
	// get collection table
	if (!lua_getmetatable(pState, -1))
		return;

	lua_newtable(pState);
	lua_newtable(pState);
	lua_pushliteral(pState,"v");
	lua_setfield(pState,-2,"__mode");
	lua_setmetatable(pState, -2);
	lua_setfield(pState, -2, "|items");

	lua_pushnumber(pState, 0);
	lua_setfield(pState, -2, "|count");

	lua_pop(pState, 1);
}


void Script::ClearCollectionParameters(lua_State* pState, const char* field,const Parameters& parameters) {
	// index -1 must be the collection
	if (!lua_getmetatable(pState, -1))
		return;
	string buffer;

	lua_getfield(pState, -1, String::Format(buffer,"|",field,"OnChange").c_str());
	Parameters::OnChange::Type* pOnChange((Parameters::OnChange::Type*)lua_touserdata(pState, -1));
	lua_pop(pState, 1);
	lua_pushnil(pState);
	lua_setfield(pState, -2, buffer.c_str());

	lua_getfield(pState, -1, String::Format(buffer,"|",field,"OnClear").c_str());
	Parameters::OnClear::Type* pOnClear((Parameters::OnClear::Type*)lua_touserdata(pState, -1));
	lua_pop(pState, 1);
	lua_pushnil(pState);
	lua_setfield(pState, -2, buffer.c_str());

	lua_pop(pState, 1); // metatable

	if (pOnChange) {
		parameters.OnChange::unsubscribe(*pOnChange);
		delete pOnChange;
	}
	if (pOnClear) {
		parameters.OnClear::unsubscribe(*pOnClear);
		delete pOnClear;
	}
}

int Script::Item(lua_State *pState) {
	SCRIPT_BEGIN(pState)
		SCRIPT_ERROR("This collection doesn't implement call operator, use [] operator rather")
	SCRIPT_END
	return 0;
}


int Script::IndexCollection(lua_State* pState) {
	// 1 table
	// 2 key

	if (lua_getmetatable(pState, 1)) {

		lua_getfield(pState, -1, "|items");
		// |items

		lua_pushvalue(pState, 2); // key
		lua_gettable(pState, -2);
		// result!

		if (lua_isnil(pState, -1)) { // if no result
			// check sub key
			lua_pushvalue(pState, 2); // key
			lua_gettable(pState, -4); // table
			lua_replace(pState, -2);
		}

		lua_replace(pState, -2);
		lua_replace(pState, -2);
	} else
		lua_pushnil(pState);


	if (lua_isnil(pState, -1)) {
		const char* name = lua_tostring(pState,2);
		if (name && strcmp(name, "count") == 0) {
			lua_pushnumber(pState, lua_objlen(pState, 1));
			lua_replace(pState, -2);
		}
	}

	return 1;
}

int Script::LenCollection(lua_State* pState) {
	// 1 table
	if(lua_getmetatable(pState, 1)) {
		lua_getfield(pState, -1, "|count");
		if (lua_isnumber(pState, -1)) {
			lua_replace(pState, -2);
			return 1;
		}
		lua_pop(pState, 2);
	}
	lua_pushnumber(pState,lua_objlen(pState, 1));
	return 1;
}

int Script::CollectionToString(lua_State* pState) {
	// 1 - table
	lua_getfield(pState, 1, "__tostring");
	if (!lua_isstring(pState, -1)) {
		lua_pop(pState, 1); 
		std::string buffer("Collection_");
		lua_pushstring(pState,String::Append(buffer, lua_topointer(pState, 1)).c_str());
	}
	return 1;
}


int Script::Next(lua_State* pState) {
	// 1 table
	// [2 key] (optional)
	if (lua_gettop(pState) < 2)
		lua_pushnil(pState);
	return Script::Next(pState, 1);
}

int Script::Next(lua_State* pState,int index) {
	// -1 is key
	if (!lua_istable(pState, index)) {
		SCRIPT_BEGIN(pState)
			SCRIPT_ERROR("next on a ", lua_typename(pState,lua_type(pState, 1)), " value")
		SCRIPT_END
		return 0;
	}
	if (lua_getmetatable(pState,  index)) {
		lua_getfield(pState, -1, "|items");
		lua_replace(pState, -2);
		if (!lua_istable(pState, -1))
			lua_pop(pState, 1);
		else
			lua_replace(pState, index);
	};
	int results = lua_next(pState, index);
	if (results>0)
		++results;
	return results;
}

static int INext(lua_State* pState) {
	// 1 table
	// [2 index] (optional,start to 1)
	if (!lua_istable(pState, 1)) {
		SCRIPT_BEGIN(pState)
			SCRIPT_ERROR("inext on a ", lua_typename(pState,lua_type(pState, 1)), " value")
		SCRIPT_END
		return 0;
	}
	if (lua_getmetatable(pState,  1)) {
		lua_getfield(pState, -1, "|items");
		lua_replace(pState, -2);
		if (!lua_istable(pState, -1))
			lua_pop(pState, 1);
		else
			lua_replace(pState, 1);
	};
	if (lua_gettop(pState) < 2)
		lua_pushnumber(pState,1);
	else
		lua_pushnumber(pState, lua_tonumber(pState,2)+1);
	lua_pushvalue(pState, -1);
	lua_gettable(pState, 1);
	if (lua_isnil(pState, -1)) {
		lua_replace(pState, -2);
		return 1;
	}
	return 2;
}

int Script::Pairs(lua_State* pState) {
	// 1 table
	if (!lua_istable(pState, 1)) {
		SCRIPT_BEGIN(pState)
			SCRIPT_ERROR("pairs on a ", lua_typename(pState,lua_type(pState, 1)), " value")
		SCRIPT_END
		return 0;
	}
	lua_pushcfunction(pState,&Next);
	lua_pushvalue(pState, 1);
	return 2;
}

int Script::IPairs(lua_State* pState) {
	// 1 table
	if (!lua_istable(pState, 1)) {
		SCRIPT_BEGIN(pState)
			SCRIPT_ERROR("ipairs on a ", lua_typename(pState,lua_type(pState, 1)), " value")
		SCRIPT_END
		return 0;
	}
	lua_pushcfunction(pState,&INext);
	lua_pushvalue(pState, 1);
	return 2;
}


const UInt8* Script::ToId(const UInt8* data, UInt32 size, UInt8 id[64]) {
	if (size == 64)
		return data;
	if (size == 32) {
		Buffer buffer(id, 64);
		return  Util::FormatHex(data, size, buffer).data();
	}
	return NULL;
}

const UInt8* Script::ToRawId(const UInt8* data, UInt32 size, UInt8 rawID[32]) {
	if (size == 32)
		return data;
	if (size == 64) {
		Buffer buffer(rawID, 32);
		return  Util::UnformatHex(data, size, buffer).data();
	}
	return NULL;
}

const char* Script::ToPrint(lua_State* pState, string& out) {
	int top = lua_gettop(pState);
	int args = 0;
	while (args++ < top)
		ToString(pState,args,out);
	return out.c_str();
}

string& Script::ToString(lua_State* pState, int index, string& out) {
	int type = lua_type(pState, index);
	switch (type) {
		case LUA_TTABLE: {
			if (lua_getmetatable(pState, index)) {
				lua_getfield(pState, -1, "__tostring");
				lua_replace(pState, -2);
				if (lua_isfunction(pState, -1)) {
					lua_pushvalue(pState, index);
					lua_call(pState, 1, 1);
					if (lua_isstring(pState, -1)) {
						out += lua_tostring(pState,-1);
						lua_pop(pState, 1);
						break;
					}
				}
				lua_pop(pState, 1);
			}
		}
		default:
			String::Append(out, lua_typename(pState,type), "_", lua_topointer(pState, index));
			break;
		case LUA_TBOOLEAN: {
			out += lua_toboolean(pState,index) ? "(true)" : "(false)";
			break;
		}
		case LUA_TNIL: {
			out += "(null)";
			break;
		}
		case LUA_TNUMBER:
		case LUA_TSTRING:
			out += lua_tostring(pState,index);
			break;
	}
	return out;
}


void Script::Test(lua_State* pState) {
	int i;
	int top = lua_gettop(pState);

	printf("total in stack %d\n", top);

	for (i = 1; i <= top; i++) {  /* repeat for each level */
		int t = lua_type(pState, i);
		switch (t) {

		case LUA_TNONE:  /* strings */
			printf("none\n");
			break;
		case LUA_TNIL:  /* strings */
			printf("nil\n");
			break;
		case LUA_TSTRING:  /* strings */
			printf("string: '%s'\n", lua_tostring(pState, i));
			break;
		case LUA_TBOOLEAN:  /* booleans */
			printf("boolean %s\n", lua_toboolean(pState, i) ? "true" : "false");
			break;
		case LUA_TNUMBER:  /* numbers */
			printf("number: %g\n", lua_tonumber(pState, i));
			break;
		default:  /* other values */
			printf("%s: %p\n", lua_typename(pState, t),lua_topointer(pState,i));
			break;
		}
		printf("  ");  /* put a separator */
	}
	printf("\n");  /* end the listing */

}

