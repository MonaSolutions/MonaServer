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


#include "Mona/DataReader.h"
#include "Mona/DataWriter.h"
#include "Mona/Logs.h"
#include <cstring>
#include <map>


extern "C" {
	#include "luajit-2.0/lua.h"
	#include "luajit-2.0/lauxlib.h"
}


#define SCRIPT_FILE(DEFAULT)					(strlen(Script::LuaDebug.short_src)>0 && strcmp(Script::LuaDebug.short_src,"[C]")!=0) ? Script::LuaDebug.short_src : DEFAULT
#define SCRIPT_LINE(DEFAULT)					Script::LuaDebug.currentline>0 ? Script::LuaDebug.currentline : DEFAULT

#define SCRIPT_LOG(LEVEL,FILE,LINE,DISPLAYSCALLER,...)	 { if (Mona::Logs::GetLevel() >= LEVEL) Script::Log(__pState,LEVEL,FILE,LINE,DISPLAYSCALLER, __VA_ARGS__);}

#define SCRIPT_FATAL(...)	SCRIPT_LOG(Mona::Logger::LEVEL_FATAL,__FILE__,__LINE__, true, __VA_ARGS__)
#define SCRIPT_CRITIC(...)	SCRIPT_LOG(Mona::Logger::LEVEL_CRITIC,__FILE__,__LINE__,true, __VA_ARGS__)
#define SCRIPT_ERROR(...)	SCRIPT_LOG(Mona::Logger::LEVEL_ERROR,__FILE__,__LINE__, true, __VA_ARGS__)
#define SCRIPT_WARN(...)	SCRIPT_LOG(Mona::Logger::LEVEL_WARN,__FILE__,__LINE__, true, __VA_ARGS__)
#define SCRIPT_NOTE(...)	SCRIPT_LOG(Mona::Logger::LEVEL_NOTE,__FILE__,__LINE__, true,__VA_ARGS__)
#define SCRIPT_INFO(...)	SCRIPT_LOG(Mona::Logger::LEVEL_INFO,__FILE__,__LINE__,true, __VA_ARGS__)
#define SCRIPT_DEBUG(...)	SCRIPT_LOG(Mona::Logger::LEVEL_DEBUG,__FILE__,__LINE__, true, __VA_ARGS__)
#define SCRIPT_TRACE(...)	SCRIPT_LOG(Mona::Logger::LEVEL_TRACE,__FILE__,__LINE__, true, __VA_ARGS__)

#define SCRIPT_CALLBACK(TYPE,OBJ)								{int __args=1;lua_State* __pState = pState; bool __destructor=false; bool __thisIsConst=false; TYPE* pObj = Script::ToObject<TYPE>(__pState,__thisIsConst,true);if(!pObj) return 0; TYPE& OBJ = *pObj;int __results=lua_gettop(__pState);
#define SCRIPT_DESTRUCTOR_CALLBACK(TYPE,OBJ)					{int __args=1;lua_State* __pState = pState; bool __destructor=true; TYPE* pObj = Script::DestructorCallback<TYPE>(__pState);if(!pObj) return 0;TYPE& OBJ = *pObj;int __results=lua_gettop(__pState);

#define SCRIPT_CALLBACK_NOTCONST_CHECK							if(__thisIsConst) {SCRIPT_ERROR("const object can't call this method") return 0;}

#define SCRIPT_CALLBACK_RETURN									__results= lua_gettop(__pState)-__results;if(__destructor && __results>0) SCRIPT_WARN("Destructor callback should return 0 arguments") return (__results>=0 ? __results : 0);}


#define SCRIPT_BEGIN(STATE)										if(lua_State* __pState = STATE) { const char* __error=NULL;

#define SCRIPT_NEW_OBJECT(TYPE,LUATYPE,OBJ)						Script::NewObject<TYPE,LUATYPE>(__pState,OBJ);

#define SCRIPT_ADD_OBJECT(TYPE,LUATYPE,OBJ)						Script::AddObject<TYPE,LUATYPE>(__pState,OBJ);
#define SCRIPT_REMOVE_OBJECT(TYPE,LUATYPE,OBJ)					Script::RemoveObject<TYPE,LUATYPE>(__pState,OBJ);

#define SCRIPT_WRITE_DATA(READER,COUNT)							Script::WriteData(__pState,READER,COUNT);
#define SCRIPT_WRITE_STRING(VALUE)								lua_pushstring(__pState,VALUE);
#define SCRIPT_WRITE_BINARY(VALUE,SIZE)							lua_pushlstring(__pState,(const char*)VALUE,SIZE);
#define SCRIPT_WRITE_BOOL(VALUE)								lua_pushboolean(__pState,VALUE);
#define SCRIPT_WRITE_FUNCTION(VALUE)							lua_pushcfunction(__pState,VALUE);
#define SCRIPT_WRITE_NUMBER(VALUE)								lua_pushnumber(__pState,(lua_Number)VALUE);
#define SCRIPT_WRITE_INT(VALUE)									lua_pushinteger(__pState,(lua_Integer)VALUE);
#define SCRIPT_WRITE_NIL										lua_pushnil(__pState);

#define SCRIPT_LAST_ERROR										__error

#define SCRIPT_MEMBER_FUNCTION_BEGIN(TYPE,OBJ,MEMBER)			{ lua_getmetatable(__pState,LUA_GLOBALSINDEX); lua_getfield(__pState,-1,"|pointers");if(!lua_isnil(__pState,-1)) {lua_replace(__pState,-2);lua_pushnumber(__pState, reinterpret_cast<unsigned>(&OBJ)); lua_gettable(__pState,-2);if(!lua_isnil(__pState,-1)) {lua_getfield(__pState,-1,MEMBER);lua_replace(__pState,-3);}} if(!lua_isfunction(__pState,-2))lua_pop(__pState,2);else {int __top=lua_gettop(__pState)-1;std::string __name = #TYPE;__name += ".";__name += MEMBER;
#define SCRIPT_MEMBER_FUNCTION_WITH_OBJHANDLE_BEGIN(TYPE,OBJ,MEMBER,OBJHANDLE)			{ lua_getmetatable(__pState,LUA_GLOBALSINDEX); lua_getfield(__pState,-1,"|pointers");if(!lua_isnil(__pState,-1)) {lua_replace(__pState,-2);lua_pushnumber(__pState, reinterpret_cast<unsigned>(&OBJ)); lua_gettable(__pState,-2); {OBJHANDLE} if(!lua_isnil(__pState,-1)) {lua_getfield(__pState,-1,MEMBER);lua_replace(__pState,-3);}} if(!lua_isfunction(__pState,-2))lua_pop(__pState,2);else {int __top=lua_gettop(__pState)-1;std::string __name = #TYPE;__name += ".";__name += MEMBER;
#define SCRIPT_FUNCTION_BEGIN(NAME)								{ bool __env=false; lua_getmetatable(__pState,LUA_GLOBALSINDEX); lua_getfield(__pState,-1,"|env"); lua_replace(__pState,-2); if(!lua_isnil(__pState,-1)) { lua_getfield(__pState,-1,NAME); __env=true;} if(!lua_isfunction(__pState,-1)) lua_pop(__pState,__env ? 2 : 1); else { if(__env) { lua_pushvalue(__pState,-2); lua_setfenv(__pState,-2); lua_replace(__pState,-2); }	int __top=lua_gettop(__pState); string __name = NAME;
#define SCRIPT_FUNCTION_CALL_WITHOUT_LOG						if(lua_pcall(__pState,lua_gettop(__pState)-__top,LUA_MULTRET,0)!=0) { __error = lua_tostring(__pState,-1); lua_pop(__pState,1); } else {--__top;int __results=lua_gettop(__pState);int __args=__top;
#define SCRIPT_FUNCTION_CALL									if(lua_pcall(__pState,lua_gettop(__pState)-__top,LUA_MULTRET,0)!=0) { SCRIPT_ERROR(__error = Script::LastError(__pState))} else {--__top;int __results=lua_gettop(__pState);int __args=__top;
#define SCRIPT_FUNCTION_NULL_CALL								{ lua_pop(__pState,lua_gettop(__pState)-__top+1);--__top;int __results=lua_gettop(__pState);int __args=__top;
#define SCRIPT_FUNCTION_END										lua_pop(__pState,__results-__top);__args = __results-__args; if(__args>0) SCRIPT_WARN(__args," arguments not required on '",__name,"' results") else if(__args<0) SCRIPT_WARN(-__args," missing arguments on '",__name,"' results") } } }

#define SCRIPT_CAN_READ											(__results-__args)>0
#define SCRIPT_NEXT_TYPE										(SCRIPT_CAN_READ ? lua_type(__pState,__args+1) : LUA_TNONE)
#define SCRIPT_READ_NIL											if(SCRIPT_CAN_READ) {__args++;};
#define SCRIPT_READ_BOOL(DEFAULT)								((__results-(__args++))<=0 ? DEFAULT : (lua_toboolean(__pState,__args)==0 ? false : true))
#define SCRIPT_READ_STRING(DEFAULT)								((__results-(__args++))<=0 ? DEFAULT : (lua_isstring(__pState,__args) ? lua_tostring(__pState,__args) : DEFAULT))
#define SCRIPT_READ_BINARY(VALUE,SIZE)							Mona::UInt32 SIZE = 0;const Mona::UInt8* VALUE = NULL;if((__results-(__args++))>0 && lua_isstring(__pState,__args)) { VALUE = (const Mona::UInt8*)lua_tostring(__pState,__args);SIZE = lua_objlen(__pState,__args);}
#define SCRIPT_READ_UINT(DEFAULT)								(Mona::UInt32)((__results-(__args++))<=0 ? DEFAULT : (lua_isnumber(__pState,__args) ? (Mona::UInt32)lua_tonumber(__pState,__args) : DEFAULT))
#define SCRIPT_READ_INT(DEFAULT)								(Mona::Int32)((__results-(__args++))<=0 ? DEFAULT : (lua_isnumber(__pState,__args) ? (Mona::Int32)lua_tointeger(__pState,__args) : DEFAULT))
#define SCRIPT_READ_DOUBLE(DEFAULT)								(double)((__results-(__args++))<=0 ? DEFAULT : (lua_isnumber(__pState,__args) ? lua_tonumber(__pState,__args) : DEFAULT))
#define SCRIPT_READ_DATA(WRITER)								Script::ReadData(__pState,WRITER,__results-__args);__args=__results;
#define SCRIPT_READ_NEXT										__args++;

#define SCRIPT_END												}


class Script {
public:

	static const char*	LastError(lua_State *pState);
	static void			Test(lua_State *pState);

	static void			WriteData(lua_State *pState,Mona::DataReader& reader,Mona::UInt32 count);
	static void			ReadData(lua_State *pState,Mona::DataWriter& writer,Mona::UInt32 count);

	static void			CloseState(lua_State* pState);
	static lua_State*	CreateState();
;

	template<class CollectorType = Script, class LUAItemType = Script>
	static bool Collection(lua_State* pState, int index,const char* field, Mona::UInt32 size, CollectorType* pCollector = NULL) {
		lua_getmetatable(pState, index);
		// update count
		lua_pushnumber(pState, size);
		lua_setfield(pState, -2, "|count");

		bool creation(false);
		// get collection table
		lua_getfield(pState, -1, field);
		if (!lua_istable(pState, -1)) {
			lua_pop(pState, 1);
			// create
			lua_newtable(pState);
			// metatable
			lua_newtable(pState);
			lua_pushvalue(pState, -1);
			lua_pushcfunction(pState, &Script::NewIndexProhibited);
			lua_setfield(pState, -2, "__newindex");
			lua_pushcfunction(pState, &Script::Len);
			lua_setfield(pState, -2, "__len");
			lua_pushstring(pState, "change metatable of datatable values is prohibited");
			lua_setfield(pState, -2, "__metatable");
			lua_setmetatable(pState, -3);
			
			lua_pushvalue(pState, -2);
			lua_setfield(pState, -4, field);
			creation = true;
		} else
			lua_getmetatable(pState, -1);

		if (pCollector) {
			lua_pushlightuserdata(pState, pCollector);
			lua_setfield(pState, -2, "|collector");
			lua_pushcfunction(pState, &LUAItemType::Item);
			lua_setfield(pState, -2, "__call");
		}

		lua_pop(pState, 1); // remove metatable of collection
		lua_replace(pState, -2); // collection replace first metatable
		return creation;
	}

	template<class CollectorType>
	static CollectorType* GetCollector(lua_State *pState, int index) {
		CollectorType* pCollector = NULL;
		if (lua_getmetatable(pState, index)) {
			lua_getfield(pState, -1, "|collector");
			pCollector = (CollectorType*)lua_touserdata(pState, -1);
			lua_pop(pState, 2);
		}
		return pCollector;
	}

	template<class Type,class LUAType>
	static void NewObject(lua_State *pState, Type& object) {
		CreateObject<Type,LUAType>(pState,object);

		// add desctructor
		lua_newuserdata(pState, sizeof(void*));
		lua_getmetatable(pState, -2);
		lua_pushnumber(pState, 1);
		lua_setfield(pState, -2, "|var"); // never const
		lua_pushcfunction(pState, &LUAType::Destroy);
		lua_setfield(pState, -2, "__gc"); // function in metatable
		lua_pushvalue(pState, -2);
		lua_setfield(pState, -2, "|gcThis"); // userdata in metatable
		lua_setmetatable(pState, -2); // metatable of user data, as metatable as object
		lua_pop(pState, 1);

		LUAType::Init(pState, object);
	}

	template<class Type, class LUAType>
	static void AddObject(lua_State *pState, Type& object) {
		AddObject<Type, LUAType>(pState, (const Type&)object);
		// add //var
		lua_getmetatable(pState,-1);
		lua_pushnumber(pState,1);
		lua_setfield(pState,-2,"|var");
		lua_pop(pState,1);
	}

	template<class Type,class LUAType>
	static void AddObject(lua_State *pState, const Type& object) {
		lua_getmetatable(pState, LUA_GLOBALSINDEX);
		lua_getfield(pState, -1,"|objects");
		if (!lua_istable(pState, -1)) {
			lua_pop(pState, 1);
			lua_newtable(pState);
			lua_pushvalue(pState, -1);
			lua_setfield(pState, -3, "|objects");
		}
		lua_replace(pState, -2);
		double ptr(0);
		ptr = (double)reinterpret_cast<unsigned>(&object);
		lua_pushnumber(pState, ptr);
		lua_gettable(pState, -2);
		bool creation(false);
		if(!lua_istable(pState,-1)) {
			lua_pop(pState,1);
			CreateObject<Type, LUAType>(pState, object);
			lua_pushnumber(pState, ptr);
			lua_pushvalue(pState,-2);
			lua_settable(pState,-4);
			creation = true;
		}
		lua_replace(pState, -2);
		
		lua_getmetatable(pState,-1);
		// remove //var
		lua_pushnil(pState);
		lua_setfield(pState,-2,"|var");
		lua_pop(pState, 1);

		if (creation)
			LUAType::Init(pState, (Type&)object);
	}

	template<class Type, class LUAType>
	static void RemoveObject(lua_State* pState, const Type& object) {
		ClearObject<Type, LUAType>(pState, object, true);
		LUAType::Clear(pState, object);
	}

	template<class Type, class LUAType>
	static void RemoveObject(lua_State* pState, int index) {
		if (lua_getmetatable(pState, index)) {
			// remove this
			lua_pushnil(pState);
			lua_setfield(pState, -2, "|this");
			lua_pop(pState, 1);
		}
		lua_pushvalue(pState, index);
		bool isConst;
		Type* pObject = ToObject<Type>(pState, isConst);
		lua_pop(pState, 1);
		if (pObject) {
			ClearObject<Type, LUAType>(pState, *pObject, false);
			LUAType::Clear(pState, *pObject);
		}
	}


	template<class Type, class LUAType>
	static void ClearObject(lua_State *pState, const Type& object) {
		ClearObject<Type, LUAType>(pState, object, true);
	}


	template<class Type>
	static Type* DestructorCallback(lua_State *pState) {
		if(lua_gettop(pState)==0 || !lua_isuserdata(pState,1)) {
			SCRIPT_BEGIN(pState)
				SCRIPT_ERROR("Bad 'this' argument, bad destructor declaration")
			SCRIPT_END
			return NULL;
		}

		bool isConst;
		return ToObject<Type>(pState,isConst,true);
	}

	template<class Type>
	static bool FromObject(lua_State* pState, const Type& object) {
		lua_getmetatable(pState, LUA_GLOBALSINDEX);
		lua_getfield(pState, -1,"|pointers");
		lua_replace(pState, -2);
		if (!lua_istable(pState, -1)) {
			lua_pop(pState, 1);
			return false;
		}
		lua_pushnumber(pState, (double)reinterpret_cast<unsigned>(&object));
		lua_gettable(pState, -2);
		if (!lua_istable(pState, -1)) {
			lua_pop(pState, 1);
			return false;
		}
		return true;
	}

	template<class Type>
	static Type* ToObject(lua_State *pState,bool& isConst,bool callback=false) {
		if(lua_gettop(pState)==0) {
			if(!callback) return NULL;
			SCRIPT_BEGIN(pState)
				SCRIPT_ERROR("'this' argument not present, call method with ':' colon operator")
			SCRIPT_END
			return NULL;
		}

		if(lua_getmetatable(pState,callback ? 1 : -1)==0) {
			if(!callback) return NULL;
			SCRIPT_BEGIN(pState)
				SCRIPT_ERROR("'this' argument has no metatable, call method with ':' colon operator")
			SCRIPT_END
			return NULL;
		}

		// already deleted?
		lua_getfield(pState,-1,"|this");
		Type* pThis = (Type*)lua_touserdata(pState, -1);
		if (!pThis) {
			lua_pop(pState,2);
			if(!callback) return NULL;
			SCRIPT_BEGIN(pState)
				SCRIPT_ERROR("Object deleted")
			SCRIPT_END
			return NULL;
		}
		lua_pop(pState,1);

		// isConst?
		lua_getfield(pState,-1,"|var");
		if(lua_isnil(pState,-1))
			isConst=true;

		lua_pop(pState,2);

		return pThis;
	}


	template <typename ...Args>
	static void Log(lua_State* pState, Mona::Logger::Level level, const char* file, long line, bool displaysCaller, Args&&... args) {
		// I stop remonting level stack at the sub level where I have gotten the name
		int stack = 0;
		bool nameGotten(false);
		while (lua_getstack(pState, stack++, &Script::LuaDebug) == 1) {
			lua_getinfo(pState, Script::LuaDebug.name ? "Sl" : "nSl", &Script::LuaDebug);
			if (nameGotten)
				break;
			if (Script::LuaDebug.name)
				nameGotten = true;
		}
		if (displaysCaller && Script::LuaDebug.name) {
			if (Script::LuaDebug.namewhat)
				Mona::Logs::Log(level, SCRIPT_FILE(file), SCRIPT_LINE(line), "(", Script::LuaDebug.namewhat, " '", Script::LuaDebug.name, "') ", args ...);
			else
				Mona::Logs::Log(level, SCRIPT_FILE(file), SCRIPT_LINE(line), "(", Script::LuaDebug.name, ") ", args ...);
		} else
			Mona::Logs::Log(level, SCRIPT_FILE(file), SCRIPT_LINE(line), args ...);
		Script::LuaDebug.name = Script::LuaDebug.namewhat = NULL;
		if (Script::LuaDebug.short_src)
			Script::LuaDebug.short_src[0] = '\0';
		Script::LuaDebug.currentline = 0;
	}

	static lua_Debug	LuaDebug;

private:
	static const char* ToPrint(lua_State* pState,std::string& out);
	static void	ReadData(lua_State *pState,Mona::DataWriter& writer,Mona::UInt32 count,std::map<Mona::UInt64,Mona::UInt32>& references);
	static void WriteData(lua_State *pState,Mona::DataReader::Type type,Mona::DataReader& reader);


	static int NewIndexProhibited(lua_State *pState) {
		// prohibited all users additions!
		return 0;
	}

	template<class Type,class LUAType>
	static void CreateObject(lua_State *pState, const Type& object) {
		lua_getmetatable(pState, LUA_GLOBALSINDEX);

		// Create table to represent our object
		lua_newtable(pState); 

		// metatable
		lua_newtable(pState); 
	
		// //this
		lua_pushlightuserdata(pState,(void*)&object);
		lua_setfield(pState,-2,"|this");

		// call => override operator ( )
		lua_pushcfunction(pState, &Script::Call<LUAType>);
		lua_setfield(pState,-2,"__call");

		// len => override operator #
		lua_pushcfunction(pState, &Script::Len);
		lua_setfield(pState, -2, "__len");

		// pairs => override operator pairs
		lua_pushcfunction(pState, &Script::Pairs);
		lua_setfield(pState, -2, "__pairs");

		// ipairs => override operator ipairs
		lua_pushcfunction(pState, &Script::Pairs);
		lua_setfield(pState, -2, "__ipairs");

		// get
		lua_pushcfunction(pState,&LUAType::Get);
		lua_setfield(pState,-2,"__index");

		// set
		lua_pushcfunction(pState,&LUAType::Set);
		lua_setfield(pState,-2,"__newindex");

		// hide metatable
		lua_pushstring(pState,"change metatable of this object is prohibited");
		lua_setfield(pState,-2,"__metatable");

		lua_setmetatable(pState,-2);

		// record the pointer
		lua_getfield(pState,-2,"|pointers");
		if(lua_isnil(pState,-1)) {
			lua_pop(pState,1);
			lua_newtable(pState);
			lua_pushvalue(pState,-1);
			lua_newtable(pState);
			lua_pushstring(pState,"v");
			lua_setfield(pState,-2,"__mode");
			lua_setmetatable(pState,-2);
			lua_setfield(pState,-4,"|pointers");
		}
		lua_pushnumber(pState, (double)reinterpret_cast<unsigned>(&object));
		lua_pushvalue(pState, -3);
		lua_settable(pState, -3);
		lua_pop(pState,1); // remove |pointers
		lua_replace(pState, -2); // remove global metatable
	}

	template<class Type, class LUAType>
	static void ClearObject(lua_State *pState, const Type& object,bool safe) {
		lua_getmetatable(pState, LUA_GLOBALSINDEX);
		lua_getfield(pState, -1, "|objects");
		if (!lua_istable(pState, -1)) {
			lua_pop(pState, 2);
			return;
		}
		lua_replace(pState, -2);

		if (safe) {
			double ptr = (double)reinterpret_cast<unsigned>(&object);
			lua_pushnumber(pState, ptr);
			lua_gettable(pState, -2);
			if (lua_istable(pState, -1)) {
				// remove this
				if (lua_getmetatable(pState, -1)) {
					lua_pushnil(pState);
					lua_setfield(pState, -2, "|this");
					lua_pop(pState, 1);
				}

				// erase id entry object
				lua_pushnumber(pState, ptr);
				lua_pushnil(pState);
				lua_settable(pState, -4);
			}
			lua_pop(pState, 1);
		} else {
			lua_pushnumber(pState, (double)reinterpret_cast<unsigned>(&object));
			lua_pushnil(pState);
			lua_settable(pState, -3);
		}

		lua_pop(pState, 1);
	}

	template<class LUAType>
	static int Call(lua_State* pState) {
		lua_pushstring(pState, "(");
		lua_insert(pState, 2);
		int result = LUAType::Get(pState);
		lua_remove(pState, 2);
		return result;
	}

	static int Len(lua_State* pState);
	static int Pairs(lua_State* pState);

	static int Item(lua_State *pState) { return 0; }

	static int Error(lua_State* pState);
	static int Warn(lua_State* pState);
	static int Note(lua_State* pState);
	static int Info(lua_State* pState);
	static int Debug(lua_State* pState);
	static int Trace(lua_State* pState);

	static int	Panic(lua_State *pState);

};
