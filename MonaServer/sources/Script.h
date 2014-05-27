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
#define SCRIPT_CALLBACK_FIX_INDEX(NAME)							{ lua_pushstring(__pState,NAME); lua_pushvalue(__pState, -2); lua_rawset(__pState, 1);}

#define SCRIPT_CALLBACK_RETURN									__results= lua_gettop(__pState)-__results;if(__destructor && __results>0) SCRIPT_WARN("Destructor callback should return 0 arguments") return (__results>=0 ? __results : 0);}


#define SCRIPT_BEGIN(STATE)										if(lua_State* __pState = STATE) { const char* __error=NULL;

#define SCRIPT_NEW_OBJECT(TYPE,LUATYPE,OBJ)						Script::NewObject<TYPE,LUATYPE>(__pState,*OBJ);

#define SCRIPT_ADD_OBJECT(TYPE,LUATYPE,OBJ)						Script::AddObject<TYPE,LUATYPE>(__pState,OBJ);
#define SCRIPT_REMOVE_OBJECT(TYPE,LUATYPE,OBJ)					Script::RemoveObject<TYPE,LUATYPE>(__pState,OBJ);

#define SCRIPT_WRITE_DATA(READER,COUNT)							Script::WriteData(__pState,READER,COUNT);
#define SCRIPT_WRITE_STRING(VALUE)								lua_pushstring(__pState,VALUE);
#define SCRIPT_WRITE_BINARY(VALUE,SIZE)							lua_pushlstring(__pState,(const char*)VALUE,SIZE);
#define SCRIPT_WRITE_BOOL(VALUE)								lua_pushboolean(__pState,VALUE);
#define SCRIPT_WRITE_FUNCTION(VALUE)							lua_pushcfunction(__pState,&VALUE);
#define SCRIPT_WRITE_NUMBER(VALUE)								lua_pushnumber(__pState,(lua_Number)VALUE);
#define SCRIPT_WRITE_INT(VALUE)									lua_pushinteger(__pState,(lua_Integer)VALUE);
#define SCRIPT_WRITE_NIL										lua_pushnil(__pState);

#define SCRIPT_LAST_ERROR										__error

#define SCRIPT_MEMBER_FUNCTION_BEGIN(TYPE,OBJ,MEMBER)			if(Script::FromObject<TYPE>(__pState,OBJ)) { lua_pushstring(__pState,MEMBER); lua_rawget(__pState,-2); lua_insert(__pState,-2); if(!lua_isfunction(__pState,-2)) lua_pop(__pState,2); else { int __top=lua_gettop(__pState)-1; const char* __name = #TYPE"."#MEMBER;
#define SCRIPT_MEMBER_FUNCTION_WITH_OBJHANDLE_BEGIN(TYPE,OBJ,MEMBER,OBJHANDLE)			if(Script::FromObject<TYPE>(__pState,OBJ)) { {OBJHANDLE} lua_pushstring(__pState,MEMBER); lua_rawget(__pState,-2); lua_insert(__pState,-2); if(!lua_isfunction(__pState,-2)) lua_pop(__pState,2); else { int __top=lua_gettop(__pState)-1; const char* __name = #TYPE"."#MEMBER;
#define SCRIPT_FUNCTION_BEGIN(NAME)								{ lua_getmetatable(__pState,LUA_GLOBALSINDEX); lua_getfield(__pState,-1,"|env"); lua_replace(__pState,-2); lua_getfield(__pState,lua_isnil(__pState,-1) ? LUA_GLOBALSINDEX : -1,NAME); lua_replace(__pState,-2); if(!lua_isfunction(__pState,-1)) lua_pop(__pState,1); else { int __top=lua_gettop(__pState); const char* __name = NAME;
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
#define SCRIPT_READ_DATA(WRITER)								Script::ReadData(__pState,WRITER,__results-__args).endWrite(); __args=__results;
#define SCRIPT_READ_NEXT										__args++;

#define SCRIPT_END												}


class Script {
public:

	static const char*	LastError(lua_State *pState);
	static void			Test(lua_State *pState);

	static Mona::DataReader& WriteData(lua_State *pState,Mona::DataReader& reader,Mona::UInt32 count=0);
	static Mona::DataWriter& ReadData(lua_State *pState,Mona::DataWriter& writer,Mona::UInt32 count);

	static void			CloseState(lua_State* pState);
	static lua_State*	CreateState();

	static int Next(lua_State* pState);
	static int Pairs(lua_State* pState);
	static int IPairs(lua_State* pState);

	static void PushValue(lua_State* pState, const std::string& value) { PushValue(pState, value.c_str()); }
	static void PushValue(lua_State* pState,const char* value);
	static void PushValue(lua_State* pState,const Mona::UInt8* value, Mona::UInt32 size);

	template <typename ...Args>
	static void PushKeyValue(lua_State* pState, const char* key, Args&&... args) { lua_pushstring(pState, key); PushValue(pState,args ...); }
	template <typename ...Args>
	static void PushKeyValue(lua_State* pState, const std::string& key, Args&&... args) { lua_pushstring(pState, key.c_str()); PushValue(pState,args ...); }

	static void ClearCollectionParameters(lua_State* pState,const char* field, const Mona::Parameters& parameters);

	static void FillCollection(lua_State* pState, Mona::UInt32 size);
	static void ClearCollection(lua_State* pState);

	template<class CollectorType = Script, class LUAItemType = CollectorType>
	static bool Collection(lua_State* pState, int index,const char* field, CollectorType* pCollector = NULL) {
		if (!field) {
			lua_pushnil(pState);
			return false;
		}
		if (!lua_getmetatable(pState, index)) {
			SCRIPT_BEGIN(pState)
				SCRIPT_ERROR("Invalid ", field, " collection ",index," index, no metatable")
			SCRIPT_END
			return false;
		}

		bool creation(false);

		// get collection table
		lua_getfield(pState, -1, field);
		if (!lua_istable(pState, -1)) {
			lua_pop(pState, 1);

			creation = true;

			// create table
			lua_newtable(pState);

			// metatable
			lua_newtable(pState);

			lua_pushcfunction(pState, &Script::Len);
			lua_setfield(pState, -2, "__len");

			lua_pushcfunction(pState, &Script::IndexCollection);
			lua_setfield(pState, -2, "__index");

			lua_pushcfunction(pState, &Script::NewIndexCollection);
			lua_setfield(pState, -2, "__newindex");

			lua_pushnumber(pState, 0);
			lua_setfield(pState, -2, "|count");

#if !defined(_DEBUG)
			lua_pushstring(pState, "change metatable of datatable values is prohibited");
			lua_setfield(pState, -2, "__metatable");
#endif
			lua_setmetatable(pState, -2);

			lua_pushvalue(pState, -1);
			lua_setfield(pState, -3, field);
		}

		if (pCollector) {
			lua_getmetatable(pState, -1);
			lua_pushlightuserdata(pState, pCollector);
			lua_setfield(pState, -2, "|collector");
			lua_pushvalue(pState, index<0 ? (index-3) : index);
			lua_setfield(pState, -2, "|self"); // to remove self on call!
			lua_pushcfunction(pState, &Script::ItemSafe<LUAItemType>);
			lua_setfield(pState, -2, "__call");
			lua_pop(pState, 1);
		}

		lua_replace(pState, -2); // collection replace metatable
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

	
	template<class CollectorType = Script, class LUAItemType = CollectorType,class ObjectType>
	static void InitCollectionParameters(lua_State* pState, ObjectType& object,const char* field, const Mona::Parameters& parameters, CollectorType* pCollector = NULL) {
		// index -1 must be the collector
		Mona::Parameters::OnChange::Type* pOnChange = new Mona::Parameters::OnChange::Type([pState,&object,&parameters,field](const std::string& key, const char* value) {
			if (Script::FromObject(pState, object)) {
				Script::Collection(pState, -1, field);
				lua_pushstring(pState, key.c_str());
				if (value)
					lua_pushstring(pState, value);
				else
					lua_pushnil(pState);
				Script::FillCollection(pState, 1);
				lua_pop(pState, 2);
			}
		});
		Mona::Parameters::OnClear::Type* pOnClear = new Mona::Parameters::OnClear::Type([pState,&object,field]() {
			if (Script::FromObject(pState, object)) {
				Script::Collection(pState, -1, field);
				Script::ClearCollection(pState);
				lua_pop(pState, 2);
			}
		});

		lua_getmetatable(pState, -1);
		std::string buffer;
		lua_pushlightuserdata(pState, pOnChange);
		lua_setfield(pState, -2, Mona::String::Format(buffer,"|",field,"OnChange").c_str());
		lua_pushlightuserdata(pState, pOnClear);
		lua_setfield(pState, -2, Mona::String::Format(buffer,"|",field,"OnClear").c_str());
		lua_pop(pState, 1);

		Script::Collection<CollectorType,LUAItemType>(pState, -1, field, pCollector);
		Mona::Parameters::ForEach forEach([pState](const std::string& key, const std::string& value) {
			Script::PushKeyValue(pState, key, value);
		});
		Script::FillCollection(pState, parameters.iterate(forEach));
		lua_setfield(pState, -2,field);

		parameters.OnChange::subscribe(*pOnChange);
		parameters.OnClear::subscribe(*pOnClear);
	}


	template<class Type,class LUAType>
	static void NewObject(lua_State *pState, Type& object) {
		Pointer<Type, LUAType>(pState, object);
		// add desctructor
		lua_newuserdata(pState, sizeof(void*));
		lua_getmetatable(pState, -2);
		lua_pushcfunction(pState, &LUAType::Destroy);
		lua_setfield(pState, -2, "__gc"); // function in metatable
		lua_pushvalue(pState, -2);
		lua_setfield(pState, -2, "|gcThis"); // userdata in metatable
		lua_setmetatable(pState, -2); // metatable of user data, as metatable as object
		lua_pop(pState, 1);
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
		lua_pushlightuserdata(pState, (void*)&object);
		lua_gettable(pState, -2);
		bool creation(false);
		if(!lua_istable(pState,-1)) {
			lua_pop(pState,1);
			Pointer<Type, LUAType>(pState, object);
			lua_pushlightuserdata(pState, (void*)&object);
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
	static void ClearObject(lua_State *pState, const Type& object) {
		ClearObject<Type, LUAType>(pState, object, true);
	}

	template<class Type, class LUAType>
	static void RemoveObject(lua_State* pState, const Type& object) {
		ClearObject<Type, LUAType>(pState, object, true);
	}

	template<class Type, class LUAType>
	static void RemoveObject(lua_State* pState, int index) {
		bool isConst;
		lua_pushvalue(pState, index);
		Type* pObject = ToObject<Type>(pState, isConst); // required table in -1 index
		if (pObject) {
			LUAType::Clear(pState, *pObject); // required table in -1 index
			ClearObject<Type, LUAType>(pState, *pObject, false);
			if (lua_getmetatable(pState, index)) {
				// remove this
				lua_pushnumber(pState, 0); // set 0 and not nil to know that this mona object is death
				lua_setfield(pState, -2, "|this");
				lua_pushnil(pState);
				lua_setfield(pState, -2, "__gc");
				lua_pop(pState, 1);
			}
		}
		lua_pop(pState, 1);
	}


	template<class Type>
	static Type* DestructorCallback(lua_State *pState) {
		if(!lua_isuserdata(pState,1)) {
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
		lua_pushlightuserdata(pState, (void*)&object);
		lua_gettable(pState, -2);
		lua_replace(pState, -2);
		if (!lua_istable(pState, -1)) {
			lua_pop(pState, 1);
			return false;
		}
		return true;
	}

	template<class Type>
	static Type* ToObject(lua_State *pState,bool& isConst,bool callback=false) {
		if(lua_getmetatable(pState,callback ? 1 : -1)==0) {
			if(!callback) return NULL;
			SCRIPT_BEGIN(pState)
				SCRIPT_ERROR("'this' argument not present or has no metatable, call method with ':' colon operator")
			SCRIPT_END
			return NULL;
		}


		// check type correspondance
		lua_getfield(pState,-1,"|type");
		if (!lua_islightuserdata(pState, -1)) {
			lua_pop(pState,2);
			if(!callback) return NULL;
			SCRIPT_BEGIN(pState)
				SCRIPT_ERROR("'this' invalid object (without type)")
			SCRIPT_END
			return NULL;
		}
		if (lua_touserdata(pState,-1)!=typeid(Type).name()) {
			lua_pop(pState,2);
			if(!callback) return NULL;
			SCRIPT_BEGIN(pState)
				SCRIPT_ERROR("'this' object doesn't match type ",typeid(Type).name())
			SCRIPT_END
			return NULL;
		}
		lua_pop(pState,1);


		// already deleted?
		lua_getfield(pState,-1,"|this");
		Type* pThis = (Type*)lua_touserdata(pState, -1);
		if (!pThis) {
			bool isDeleted(lua_isnumber(pState, -1) ? true : false);
			lua_pop(pState,2);
			if(!callback) return NULL;
			SCRIPT_BEGIN(pState)
				if (isDeleted)
					SCRIPT_ERROR(typeid(Type).name()," object deleted")
				else
					SCRIPT_ERROR(typeid(Type).name()," argument not present, call method with ':' colon operator")
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
	static Mona::DataWriter&	ReadData(lua_State *pState,Mona::DataWriter& writer,Mona::UInt32 count,std::map<Mona::UInt64,Mona::UInt32>& references);
	static Mona::DataReader&	WriteData(lua_State *pState,Mona::DataReader::Type type,Mona::DataReader& reader);

	template<class Type,class LUAType>
	static void Pointer(lua_State *pState, const Type& object) {
		lua_getmetatable(pState, LUA_GLOBALSINDEX);

		// record the pointer
		lua_getfield(pState,-1,"|pointers");
		bool creation(true);
		if(lua_isnil(pState,-1)) {
			lua_pop(pState,1);
			lua_newtable(pState);
			lua_pushvalue(pState,-1);
			lua_newtable(pState);
			lua_pushstring(pState,"v");
			lua_setfield(pState,-2,"__mode");
			lua_setmetatable(pState,-2);
			lua_setfield(pState,-3,"|pointers");
			lua_replace(pState, -2); // remove metatable
		} else {
			lua_replace(pState, -2); // remove metatable
			lua_pushlightuserdata(pState, (void*)&object);
			lua_gettable(pState, -2);
			if (lua_istable(pState, -1) && lua_getmetatable(pState,-1))
				creation = false;
			else
				lua_pop(pState, 1);
		}

		if (creation) {
			// Create table to represent our object
			lua_newtable(pState);

			// metatable
			lua_newtable(pState);
		}
	
		// |this
		lua_pushlightuserdata(pState,(void*)&object);
		lua_setfield(pState,-2,"|this");

		// |type
		lua_pushlightuserdata(pState,(void*)typeid(Type).name());
		lua_setfield(pState,-2,"|type");

		// len => override operator #
		lua_pushcfunction(pState, &Script::Len);
		lua_setfield(pState, -2, "__len");

		// get
		lua_pushcfunction(pState,&LUAType::Get);
		lua_setfield(pState,-2,"__index");

		// set
		lua_pushcfunction(pState,&LUAType::Set);
		lua_setfield(pState,-2,"__newindex");

		// hide metatable
#if !defined(_DEBUG)
		lua_pushstring(pState,"change metatable of this object is prohibited");
		lua_setfield(pState,-2,"__metatable");
#endif

		if (creation) {
			lua_setmetatable(pState, -2);

			lua_pushlightuserdata(pState, (void*)&object);
			lua_pushvalue(pState, -2);
			lua_settable(pState, -4);
		} else
			lua_pop(pState, 1); // remove metatable

		lua_replace(pState, -2); // remove |pointers
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
			lua_pushlightuserdata(pState, (void*)&object);
			lua_gettable(pState, -2);
			if (lua_istable(pState, -1)) {
				LUAType::Clear(pState, object);
				// remove this
				if (lua_getmetatable(pState, -1)) {
					lua_pushnumber(pState,0); // set 0 and not nil to know that this mona object is death
					lua_setfield(pState, -2, "|this");
					lua_pushnil(pState);
					lua_setfield(pState, -2, "__gc");
					lua_pop(pState, 1);
				}

				// erase id entry object
				lua_pushlightuserdata(pState, (void*)&object);
				lua_pushnil(pState);
				lua_settable(pState, -4);
			}
			lua_pop(pState, 1);
		} else {
			lua_pushlightuserdata(pState, (void*)&object);
			lua_pushnil(pState);
			lua_settable(pState, -3);
		}

		lua_pop(pState, 1);
	}

	template<class LUAItemType>
	static int ItemSafe(lua_State *pState) {
		// solve the problem of call with self collector:collection(..) in removing |self if is argument 2
		// 1 collection
		// 2 collector (self) or parameter
		if (lua_istable(pState, 2)) {
			if (lua_getmetatable(pState, 1)) {
				lua_getfield(pState, -1, "|self");
				if (lua_equal(pState, -1, 2))
					lua_remove(pState, 2);
				lua_pop(pState, 2);
			}
		}
		return LUAItemType::Item(pState);
	}
	static int Item(lua_State *pState);

	static int Len(lua_State* pState);
	static int IndexCollection(lua_State* pState);
	static int NewIndexCollection(lua_State* pState);

	static int Error(lua_State* pState);
	static int Warn(lua_State* pState);
	static int Note(lua_State* pState);
	static int Info(lua_State* pState);
	static int Debug(lua_State* pState);
	static int Trace(lua_State* pState);

	static int	Panic(lua_State *pState);

};
