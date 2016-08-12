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

#include "Mona/Logs.h"
#include <map>


extern "C" {
	#include "luajit-2.0/lua.h"
	#include "luajit-2.0/lauxlib.h"
}

#define SCRIPT_LOG(LEVEL,FILE,LINE,DISPLAYSCALLER,...)	 { if (Mona::Logs::GetLevel() >= LEVEL)  { std::string __buffer; Script::Log(__pState,LEVEL,FILE,LINE,DISPLAYSCALLER, __VA_ARGS__); } }

#define SCRIPT_FATAL(...)	SCRIPT_LOG(Mona::Logger::LEVEL_FATAL,__FILE__,__LINE__, true, __VA_ARGS__)
#define SCRIPT_CRITIC(...)	SCRIPT_LOG(Mona::Logger::LEVEL_CRITIC,__FILE__,__LINE__,true, __VA_ARGS__)
#define SCRIPT_ERROR(...)	SCRIPT_LOG(Mona::Logger::LEVEL_ERROR,__FILE__,__LINE__, true, __VA_ARGS__)
#define SCRIPT_WARN(...)	SCRIPT_LOG(Mona::Logger::LEVEL_WARN,__FILE__,__LINE__, true, __VA_ARGS__)
#define SCRIPT_NOTE(...)	SCRIPT_LOG(Mona::Logger::LEVEL_NOTE,__FILE__,__LINE__, true,__VA_ARGS__)
#define SCRIPT_INFO(...)	SCRIPT_LOG(Mona::Logger::LEVEL_INFO,__FILE__,__LINE__,true, __VA_ARGS__)
#define SCRIPT_DEBUG(...)	SCRIPT_LOG(Mona::Logger::LEVEL_DEBUG,__FILE__,__LINE__, true, __VA_ARGS__)
#define SCRIPT_TRACE(...)	SCRIPT_LOG(Mona::Logger::LEVEL_TRACE,__FILE__,__LINE__, true, __VA_ARGS__)

#define SCRIPT_CALLBACK(TYPE,OBJ)								{int __args=1;lua_State* __pState = pState; bool __thisIsConst=false; TYPE* pObj = Script::ToObject<TYPE>(__pState,__thisIsConst,1,true);if(!pObj) return 0; TYPE& OBJ = *pObj;int __results=lua_gettop(__pState);
#define SCRIPT_CALLBACK_TRY(TYPE,OBJ)							{int __args=1;lua_State* __pState = pState; bool __thisIsConst=false; TYPE* pObj = Script::ToObject<TYPE>(__pState,__thisIsConst,1,true);if(!pObj) return 0; TYPE& OBJ = *pObj;int __results=lua_gettop(__pState); bool __canRaise(SCRIPT_NEXT_TYPE == LUA_TFUNCTION); if(__canRaise) ++__args;

#define SCRIPT_CALLBACK_NOTCONST_CHECK							{ if(__thisIsConst) {SCRIPT_ERROR("const object can't call this method") return 0;} }
#define SCRIPT_CALLBACK_THROW(ERROR)							{ if (__canRaise) { lua_pushvalue(__pState,2); lua_pushstring(__pState,ERROR);  lua_call(pState, 1, 0); } }
#define SCRIPT_CALLBACK_RETURN									__results= lua_gettop(__pState)-__results; return (__results>=0 ? __results : 0);}

#define SCRIPT_FIX_RESULT										{ lua_pushvalue(__pState,2); lua_pushvalue(__pState, -2); lua_rawset(__pState, 1);}

#define SCRIPT_BEGIN(STATE)										if(lua_State* __pState = STATE) {

#define SCRIPT_MEMBER_FUNCTION_BEGIN(TYPE,OBJ,MEMBER)			if(Script::FromObject<TYPE>(__pState,OBJ)) { lua_pushstring(__pState, MEMBER); lua_rawget(__pState,-2); lua_insert(__pState,-2); if(!lua_isfunction(__pState,-2)) lua_pop(__pState,2); else { lua_getfenv(__pState, -2); lua_rawseti(__pState, LUA_REGISTRYINDEX, LUA_ENVIRONINDEX); int __top=lua_gettop(__pState)-1; const char* __name = #TYPE"."#MEMBER;
#define SCRIPT_FUNCTION_BEGIN(NAME,REFERENCE)					{ lua_Debug	__debug; if (REFERENCE==LUA_ENVIRONINDEX && lua_getstack(_pState, 0, &__debug)==1) lua_pushvalue(__pState, LUA_ENVIRONINDEX); else lua_rawgeti(__pState,LUA_REGISTRYINDEX, REFERENCE); if (lua_istable(__pState, -1)) { lua_getfield(__pState, -1, NAME); lua_replace(__pState, -2); } if(!lua_isfunction(__pState,-1)) lua_pop(__pState,1); else { lua_getfenv(__pState, -1); lua_rawseti(__pState, LUA_REGISTRYINDEX, LUA_ENVIRONINDEX); int __top=lua_gettop(__pState); const char* __name = NAME;
#define SCRIPT_FUNCTION_CALL_WITHOUT_LOG						const char* __error=NULL; if(lua_pcall(__pState,lua_gettop(__pState)-__top,LUA_MULTRET,0)) { __error = lua_tostring(__pState,-1); lua_pop(__pState,1); } --__top; int __results=lua_gettop(__pState); int __args=__top;
#define SCRIPT_FUNCTION_CALL									const char* __error=NULL; if(lua_pcall(__pState,lua_gettop(__pState)-__top,LUA_MULTRET,0)) SCRIPT_ERROR(__error = Script::LastError(__pState)); --__top; int __results=lua_gettop(__pState); int __args=__top;
#define SCRIPT_FUNCTION_NULL_CALL								lua_pop(__pState,lua_gettop(__pState)-__top+1);--__top;int __results=lua_gettop(__pState);int __args=__top;
#define SCRIPT_FUNCTION_ERROR									__error
#define SCRIPT_FUNCTION_END										lua_pop(__pState,__results-__top);__args = __results-__args; if(__args>0) SCRIPT_WARN(__args," arguments not required on '",__name,"' results") else if(__args<0) SCRIPT_WARN(-__args," missing arguments on '",__name,"' results") } }

#define SCRIPT_WRITE_STRING(VALUE)								lua_pushstring(__pState,VALUE);
#define SCRIPT_WRITE_BINARY(VALUE,SIZE)							lua_pushlstring(__pState,(const char*)VALUE,SIZE);
#define SCRIPT_WRITE_BOOL(VALUE)								lua_pushboolean(__pState,VALUE);
#define SCRIPT_WRITE_FUNCTION(VALUE)							lua_pushcfunction(__pState,&VALUE);
#define SCRIPT_WRITE_NUMBER(VALUE)								lua_pushnumber(__pState,(lua_Number)VALUE);
#define SCRIPT_WRITE_INT(VALUE)									lua_pushinteger(__pState,(lua_Integer)VALUE);
#define SCRIPT_WRITE_NIL										lua_pushnil(__pState);

#define SCRIPT_READ_AVAILABLE									(__results-__args)
#define SCRIPT_READ_NEXT(COUNT)									(__args+=COUNT, __args = (__args>__results) ? __results : __args)
#define SCRIPT_NEXT_TYPE										(SCRIPT_READ_AVAILABLE ? lua_type(__pState,__args+1) : LUA_TNONE)

#define SCRIPT_READ_NIL											if(__args<__results) {++__args;};
#define SCRIPT_READ_BOOL(DEFAULT)								((__results-(__args++))<=0 ? DEFAULT : (lua_toboolean(__pState,__args)==0 ? false : true))
#define SCRIPT_READ_STRING(DEFAULT)								((__results-(__args++))<=0 ? DEFAULT : (lua_isstring(__pState,__args) ? lua_tostring(__pState,__args) : DEFAULT))
#define SCRIPT_READ_BINARY(VALUE,SIZE)							Mona::UInt32 SIZE = 0;const Mona::UInt8* VALUE = NULL;if((__results-(__args++))>0 && lua_isstring(__pState,__args)) { VALUE = (const Mona::UInt8*)lua_tostring(__pState,__args);SIZE = lua_objlen(__pState,__args);}
#define SCRIPT_READ_UINT(DEFAULT)								(Mona::UInt32)((__results-(__args++))<=0 ? DEFAULT : (lua_isnumber(__pState,__args) ? (Mona::UInt32)lua_tonumber(__pState,__args) : DEFAULT))
#define SCRIPT_READ_INT(DEFAULT)								(Mona::Int32)((__results-(__args++))<=0 ? DEFAULT : (lua_isnumber(__pState,__args) ? (Mona::Int32)lua_tointeger(__pState,__args) : DEFAULT))
#define SCRIPT_READ_DOUBLE(DEFAULT)								(double)((__results-(__args++))<=0 ? DEFAULT : (lua_isnumber(__pState,__args) ? lua_tonumber(__pState,__args) : DEFAULT))							

#define SCRIPT_END												}


class Script {
public:

	static const char*	LastError(lua_State *pState);
	static void			Test(lua_State *pState);

	static const Mona::UInt8* ToRawId(const Mona::UInt8* data, Mona::UInt32 size, Mona::UInt8 rawID[32]);
	static const Mona::UInt8* ToId(const Mona::UInt8* data, Mona::UInt32 size, Mona::UInt8 id[64]);

	static void			CloseState(lua_State* pState);
	static lua_State*	CreateState();

	static int Next(lua_State* pState);
	static int Next(lua_State* pState, int index);
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

	static bool GetCollection(lua_State* pState, int index, const char* field);
	static void DeleteCollection(lua_State* pState,int index,const char* field);
	static void FillCollection(lua_State* pState, Mona::UInt32 size);
	static void ClearCollection(lua_State* pState);

	template<typename LUAItemType = Script>
	static bool Collection(lua_State* pState, int index,const char* field) {
		if (!field) {
			lua_pushnil(pState);
			return false;
		}
		if (!lua_getmetatable(pState, index)) {
			SCRIPT_BEGIN(pState)
				SCRIPT_ERROR("Invalid ", field, " collection ",index," index, no metatable")
			SCRIPT_END
			lua_pushnil(pState);
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

			lua_pushcfunction(pState, &Script::LenCollection);
			lua_setfield(pState, -2, "__len");

			lua_pushcfunction(pState, &Script::IndexCollection);
			lua_setfield(pState, -2, "__index");

			lua_pushcfunction(pState, &Script::CollectionToString);
			lua_setfield(pState, -2, "__tostring");

			lua_pushnumber(pState, 0);
			lua_setfield(pState, -2, "|count");

			lua_newtable(pState);
			lua_newtable(pState);
			lua_pushliteral(pState,"v");
			lua_setfield(pState,-2,"__mode");
			lua_setmetatable(pState, -2);
			lua_setfield(pState, -2, "|items");

			lua_pushvalue(pState, index<0 ? (index-3) : index);
			lua_setfield(pState, -2, "|self"); // to remove self on call, and get collector on __call

			lua_pushcfunction(pState, &Script::ItemSafe<LUAItemType>);
			lua_setfield(pState, -2, "__call");

#if !defined(_DEBUG)
			lua_pushliteral(pState, "change metatable of datatable values is prohibited");
			lua_setfield(pState, -2, "__metatable");
#endif
			lua_setmetatable(pState, -2);

			lua_pushvalue(pState, -1);
			lua_setfield(pState, -3, field);
		}

		lua_replace(pState, -2); // collection replace metatable
		return creation;
	}
	
	template<typename LUAItemType = Script,typename ObjectType>
	static void InitCollectionParameters(lua_State* pState, ObjectType& object,const char* field, const Mona::Parameters& parameters) {
		// index -1 must be the collector
		Mona::Parameters::OnChange::Type* pOnChange = new Mona::Parameters::OnChange::Type([pState,&object,&parameters,field](const char* key, const char* value, size_t size) {
			if (Script::FromObject(pState, object)) {
				Script::Collection(pState, -1, field);
				lua_pushstring(pState, key);
				if (value)
					lua_pushlstring(pState, value, size == std::string::npos? strlen(value) : size);
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

		Script::Collection<LUAItemType>(pState, -1, field);
		Mona::Parameters::ForEach forEach([pState](const std::string& key, const std::string& value) {
			Script::PushKeyValue(pState, key, value);
		});
		Script::FillCollection(pState, parameters.iterate(forEach));
		lua_setfield(pState, -2,field);

		parameters.OnChange::subscribe(*pOnChange);
		parameters.OnClear::subscribe(*pOnClear);
	}


	template<typename LUAType,typename Type>
	static void NewObject(lua_State *pState, Type& object) {
		CreateObject<LUAType, Type>(pState, object);
		AttachDestructor<LUAType, Type>(pState,object);
	}

	template<typename LUAType,typename Type>
	static void AddObject(lua_State *pState, Type& object) {
		lua_pushlightuserdata(pState, (void*)&object);
		lua_gettable(pState, LUA_REGISTRYINDEX);
		if (!lua_istable(pState, -1)) {
			lua_pop(pState, 1);
			if (!FromObject(pState, object)) {
				CreateObject<LUAType, Type>(pState, object);
				lua_pushlightuserdata(pState, (void*)&object);
				lua_pushvalue(pState, -2);
				lua_settable(pState, LUA_REGISTRYINDEX);
			}
		} else {
			// remove possible |const
			lua_getmetatable(pState, -1);
			lua_pushnil(pState);
			lua_setfield(pState, -2, "|const");
			lua_pop(pState, 1);
		}
	}

	template<typename LUAType,typename Type>
	static void AddObject(lua_State *pState, const Type& object) {
		lua_pushlightuserdata(pState, (void*)&object);
		lua_gettable(pState, LUA_REGISTRYINDEX);
		if (!lua_istable(pState, -1)) {
			lua_pop(pState, 1);
			if (!FromObject(pState, object)) {
				CreateObject<LUAType, Type>(pState, object);
				lua_pushlightuserdata(pState, (void*)&object);
				lua_pushvalue(pState, -2);
				lua_settable(pState, LUA_REGISTRYINDEX);
			}
		}
		//ad |const
		lua_getmetatable(pState,-1);
		lua_pushboolean(pState,true);
		lua_setfield(pState,-2,"|const"); 
		lua_pop(pState, 1);
	}

	
	template<typename LUAType,typename Type>
	static void AttachDestructor(lua_State *pState, Type& object) {
		lua_getmetatable(pState, -1);

		lua_pushcfunction(pState, &(Script::ClearObject<LUAType, Type>) );
		lua_setfield(pState, -2, "__gc"); // function in metatable

		lua_getfield(pState, -1, "|gcData");
		if (!lua_isuserdata(pState, -1)) {
			lua_newuserdata(pState, sizeof(void*));

			lua_pushvalue(pState, -3); // metatable
			lua_setmetatable(pState, -2); // metatable of user data, as metatable as object

			lua_setfield(pState, -3, "|gcData"); // userdata in metatable
		}

		lua_pop(pState, 2);
	}

	static void DetachDestructor(lua_State *pState,int index) {
		if (lua_getmetatable(pState, index)) {
			lua_pushnil(pState);
			lua_setfield(pState, -2, "__gc"); // remove destructor
			lua_pop(pState, 1); // remove metatable
		}
	}

	template<typename LUAType,typename Type>
	static void ClearObject(lua_State *pState, const Type& object) {
		// clear the object if no destructor!
		if (FromObject(pState, object)) {
			if (lua_getmetatable(pState, -1)) {
				lua_getfield(pState,-1,"__gc");
				if (!lua_isfunction(pState, -1)) {
					lua_pop(pState, 1);

					lua_pushnumber(pState,0); // set 0 and not nil to know that this mona object is death
					lua_setfield(pState, -2, "|this");

					lua_pop(pState, 1); // remove metatable
					LUAType::Clear(pState, (Type&)object);
				} else
					lua_pop(pState, 2);
			}
			lua_pop(pState, 1);
		}

		// erase id entry object
		lua_pushlightuserdata(pState, (void*)&object);
		lua_pushnil(pState);
		lua_settable(pState, LUA_REGISTRYINDEX);
	}


	template<typename Type>
	static bool FromObject(lua_State* pState, const Type& object) {
		lua_getfield(pState, LUA_REGISTRYINDEX, "|pointers");
		if (!lua_istable(pState, -1)) {
			lua_pop(pState, 1);
			return false;	
		}
		lua_pushlightuserdata(pState, (void*)&object);
		lua_gettable(pState, -2);
		if (!lua_getmetatable(pState,-1)) {
			lua_pop(pState, 2);
			return false;	
		}

		// check if deleted!
		lua_getfield(pState,-1,"|this");
		lua_replace(pState, -2); // remove metatable
		if (lua_touserdata(pState, -1) != &object) {

			// erase of |pointers
			lua_pushlightuserdata(pState, (void*)&object);
			lua_pushnil(pState);
			lua_settable(pState, -5);

			lua_pop(pState, 3);
			return false;
		}
		lua_pop(pState,1); // remove |this

		lua_replace(pState, -2); // remove |pointers
		return true;	
	}


	template<typename Type>
	static Type* ToObject(lua_State *pState,bool& isConst,int index=-1,bool withLogs=false) {
		if(lua_getmetatable(pState,index)==0) {
			if(!withLogs) return NULL;
			SCRIPT_BEGIN(pState)
				SCRIPT_ERROR("'this' argument not present or has no metatable, call method with ':' colon operator")
			SCRIPT_END
			return NULL;
		}

		// already deleted?
		lua_getfield(pState,-1,"|this");
		Type* pThis = (Type*)lua_touserdata(pState, -1);
		if (!pThis) {
			bool isDeleted(lua_isnumber(pState, -1) ? true : false);
			lua_pop(pState,2);
			if(!withLogs) return NULL;
			SCRIPT_BEGIN(pState)
				if (isDeleted)
					SCRIPT_ERROR(typeid(Type).name()," object deleted")
				else
					SCRIPT_ERROR(typeid(Type).name()," argument not present, call method with ':' colon operator")
			SCRIPT_END
			return NULL;
		}
		lua_pop(pState,1);
	
		// check type
		lua_getfield(pState,-1,"|type");
		if (lua_touserdata(pState, -1) != &typeid(Type)) {
			if(withLogs) {
				SCRIPT_BEGIN(pState)
					SCRIPT_ERROR("Cast error, type ",lua_touserdata(pState, -1)," doesn't match ",&typeid(Type), " of ",typeid(Type).name()," type")
				SCRIPT_END
			}
			lua_pop(pState,2);
			return NULL;
		}
		lua_pop(pState,1);
		
		// isConst?
		lua_getfield(pState,-1,"|const");
		isConst = lua_toboolean(pState,-1) ? true : 0;

		lua_pop(pState,2);

		return pThis;
	}

	
	template<typename Type>
	static void AddComparator(lua_State *pState) {
		lua_getmetatable(pState, -1);

		lua_pushlightuserdata(pState, (void*)&typeid(Type));
		lua_gettable(pState, LUA_REGISTRYINDEX);
		if (!lua_istable(pState, -1)) {
			lua_pop(pState, 1);
			lua_newtable(pState);

			lua_pushlightuserdata(pState, (void*)&typeid(Type));
			lua_pushvalue(pState, -2);
			lua_settable(pState, LUA_REGISTRYINDEX);

			lua_pushcfunction(pState, &Script::Le<Type>);
			lua_setfield(pState, -2, "__le");
			lua_pushcfunction(pState, &Script::Lt<Type>);
			lua_setfield(pState, -2, "__lt");
			lua_pushcfunction(pState, &Script::Eq<Type>);
			lua_setfield(pState, -2, "__eq");
		}

		lua_getfield(pState, -1, "__le");
		lua_setfield(pState, -3, "__le");
		lua_getfield(pState, -1, "__lt");
		lua_setfield(pState, -3, "__lt");
		lua_getfield(pState, -1, "__eq");
		lua_setfield(pState, -3, "__eq");

		lua_pop(pState, 2);
	}

	template <typename ...Args>
	static void Log(lua_State* pState, Mona::Logger::Level level, const char* file, long line, bool displaysCaller, Args&&... args) {
		// I stop remonting level stack at the sub level where I have gotten the name
		int stack(0);
		bool nameGotten(false);
		lua_Debug	debug;
		debug.name = NULL;
		while (lua_getstack(pState, stack++, &debug) == 1 && lua_getinfo(pState, nameGotten ? "Sl" : "nSl", &debug) && !nameGotten) {
			if (debug.name)
				nameGotten = true;
		}
		
		if (nameGotten) {
			if (strlen(debug.short_src) && strcmp(debug.short_src, "[C]"))
				file = debug.short_src;
			if (debug.currentline)
				line = debug.currentline;

			if (displaysCaller) {
				if (debug.namewhat)
					return Mona::Logs::Log(level, file, line, "(", debug.namewhat, " '", debug.name, "') ", args ...);
				return Mona::Logs::Log(level, file, line, "(", debug.name, ") ", args ...);
			}
		}
		Mona::Logs::Log(level, file, line, args ...);
	}

	

private:

	static const char*	ToPrint(lua_State* pState,std::string& out);
	static std::string&	ToString(lua_State* pState, int index, std::string& out);

	template<typename LUAType>
	static int ToString(lua_State* pState) {
		// 1 - table
		lua_getfield(pState, 1, "value");
		if (!lua_isstring(pState, -1)) {
			lua_pop(pState, 1); 
			std::string buffer(typeid(LUAType).name());
			lua_pushstring(pState,Mona::String::Append(buffer,"_", lua_topointer(pState, 1)).c_str());
		}
		return 1;
	}


	template<typename Type>
	static int Le(lua_State *pState) {
		SCRIPT_CALLBACK(Type, object)
			bool isConst;
			Type* pObject = Script::ToObject<Type>(pState, isConst,2);
			if (pObject)
				SCRIPT_WRITE_BOOL(object <= *pObject)
			else if (lua_isnil(pState, 2))
				SCRIPT_WRITE_BOOL(!object)
			else
				SCRIPT_ERROR("Comparison impossible between one ",typeid(Type).name()," and one ",lua_typename(pState,lua_type(pState,2)))
		SCRIPT_CALLBACK_RETURN
	}

	template<typename Type>
	static int Lt(lua_State *pState) {
		SCRIPT_CALLBACK(Type, object)
			bool isConst;
			Type* pObject = Script::ToObject<Type>(pState, isConst,2);
			if (pObject)
				SCRIPT_WRITE_BOOL(object < *pObject)
			else
				SCRIPT_ERROR("Comparison impossible between one ",typeid(Type).name()," and one ",lua_typename(pState,lua_type(pState,2)))
		SCRIPT_CALLBACK_RETURN
	}

	template<typename Type>
	static int Eq(lua_State *pState) {
		SCRIPT_CALLBACK(Type, object)
			bool isConst;
			Type* pObject = Script::ToObject<Type>(pState, isConst,2);
			if (pObject)
				SCRIPT_WRITE_BOOL(object == *pObject)
			else if (lua_isnil(pState, 2))
				SCRIPT_WRITE_BOOL(!object)
			else
				SCRIPT_ERROR("Comparison impossible between one ",typeid(Type).name()," and one ",lua_typename(pState,lua_type(pState,2)))
		SCRIPT_CALLBACK_RETURN
	}

	template<typename LUAType,typename Type>
	static void CreateObject(lua_State *pState, const Type& object) {

		// Create table to represent our object
		lua_newtable(pState);

		// metatable
		lua_newtable(pState);

		// |this
		lua_pushlightuserdata(pState,(void*)&object);
		lua_setfield(pState,-2,"|this");

		// |type
		lua_pushlightuserdata(pState,(void*)&typeid(Type));
		lua_setfield(pState,-2,"|type");

		// get
		lua_pushcfunction(pState,&LUAType::Get);
		lua_setfield(pState,-2,"__index");

		// toString
		lua_pushcfunction(pState, &Script::ToString<LUAType>);
		lua_setfield(pState,-2,"__tostring");

		// set
		lua_pushcfunction(pState,&LUAType::Set);
		lua_setfield(pState,-2,"__newindex");

		// hide metatable
#if !defined(_DEBUG)
		lua_pushliteral(pState,"change metatable of this object is prohibited");
		lua_setfield(pState,-2,"__metatable");
#endif

		lua_setmetatable(pState, -2);

		// record the pointer
		lua_getfield(pState,LUA_REGISTRYINDEX,"|pointers");
		if(lua_isnil(pState,-1)) {
			lua_pop(pState,1);
			lua_newtable(pState); // table
			lua_newtable(pState); // metatable

			lua_pushliteral(pState,"v");
			lua_setfield(pState,-2,"__mode");

			lua_setmetatable(pState,-2); // remove metatable

			lua_pushvalue(pState,-1); // table
			lua_setfield(pState,LUA_REGISTRYINDEX,"|pointers");
		}

		lua_pushlightuserdata(pState, (void*)&object);
		lua_pushvalue(pState, -3);
		lua_settable(pState, -3);
		lua_pop(pState,1); // remove |pointers

		LUAType::Init(pState, (Type&)object);
	}


	template<typename LUAType, typename Type>
	static int ClearObject(lua_State *pState) { // call just by __gc
		bool isConst;
		Type* pObject(ToObject<Type>(pState,isConst,1));
		if (pObject) {
			LUAType::Clear(pState, *pObject); // here it's already deleted of |objects tables (because otherwise referenceCount > 0)
			LUAType::Delete(pState, *pObject);
		}
		return 0;
	}

	template<typename LUAItemType>
	static int ItemSafe(lua_State *pState) {
		// Add the collector (self) in second arguments if not present!
		// And Remove the collection
		// 1 collection
		// 2 collector (self) or parameters
		if (lua_getmetatable(pState, 1)) {
			lua_getfield(pState, -1, "|self");
			lua_replace(pState, -2);
			if (lua_istable(pState, -1)) {
				if (!lua_equal(pState, -1, 2))
					lua_insert(pState, 2);
				lua_remove(pState, 1); // remove collection
			} else
				lua_pop(pState, 1);
		}
		return LUAItemType::Item(pState);
	}

	static int Item(lua_State *pState);
	static int CollectionToString(lua_State* pState);
	static int LenCollection(lua_State* pState);
	static int IndexCollection(lua_State* pState);


	static int Error(lua_State* pState);
	static int Warn(lua_State* pState);
	static int Note(lua_State* pState);
	static int Info(lua_State* pState);
	static int Debug(lua_State* pState);
	static int Trace(lua_State* pState);

	static int	Panic(lua_State *pState);

};
