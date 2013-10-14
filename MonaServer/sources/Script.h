/* 
	Copyright 2013 Mona - mathieu.poux[a]gmail.com
 
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
#include "Poco/Format.h"
#include "Poco/NumberFormatter.h"
#include <cstring>
#include <map>


extern "C" {
	#include "luajit-2.0/lua.h"
	#include "luajit-2.0/lauxlib.h"
}


#define SCRIPT_FILE(DEFAULT)					(strlen(Script::LuaDebug.short_src)>0 && strcmp(Script::LuaDebug.short_src,"[C]")!=0) ? Script::LuaDebug.short_src : DEFAULT
#define SCRIPT_LINE(DEFAULT)					Script::LuaDebug.currentline>0 ? Script::LuaDebug.currentline : DEFAULT

#define SCRIPT_LOG(PRIO,FILE,LINE,FMT, ...)		{ if(lua_getstack(__pState,0,&Script::LuaDebug)==1) lua_getinfo(__pState, "n", &Script::LuaDebug); \
												if(lua_getstack(__pState,1,&Script::LuaDebug)==1) lua_getinfo(__pState, "Sl", &Script::LuaDebug); \
												if(!SCRIPT_LOG_NAME_DISABLED && Script::LuaDebug.name) { \
													if(Script::LuaDebug.namewhat) { \
														LOG(PRIO,SCRIPT_FILE(FILE),SCRIPT_LINE(LINE),"(%s '%s') "FMT,Script::LuaDebug.namewhat,Script::LuaDebug.name,## __VA_ARGS__) \
													} else { \
														LOG(PRIO,SCRIPT_FILE(FILE),SCRIPT_LINE(LINE),"('%s') "FMT,Script::LuaDebug.name,## __VA_ARGS__)} \
												} else \
													LOG(PRIO,SCRIPT_FILE(FILE),SCRIPT_LINE(LINE),FMT,## __VA_ARGS__) \
												Script::LuaDebug.name = Script::LuaDebug.namewhat = NULL; \
												if(Script::LuaDebug.short_src) Script::LuaDebug.short_src[0]='\0'; \
												Script::LuaDebug.currentline=0;}

#define SCRIPT_LOG_NAME_DISABLED	false
#define SCRIPT_FATAL(FMT, ...)		SCRIPT_LOG(Mona::Logger::PRIO_FATAL,__FILE__,__LINE__,FMT, ## __VA_ARGS__)
#define SCRIPT_CRITIC(FMT, ...)		SCRIPT_LOG(Mona::Logger::PRIO_CRITIC,__FILE__,__LINE__,FMT, ## __VA_ARGS__)
#define SCRIPT_ERROR(FMT, ...)		SCRIPT_LOG(Mona::Logger::PRIO_ERROR,__FILE__,__LINE__,FMT, ## __VA_ARGS__)
#define SCRIPT_WARN(FMT, ...)		SCRIPT_LOG(Mona::Logger::PRIO_WARN,__FILE__,__LINE__,FMT, ## __VA_ARGS__)
#define SCRIPT_NOTE(FMT, ...)		SCRIPT_LOG(Mona::Logger::PRIO_NOTE,__FILE__,__LINE__,FMT, ## __VA_ARGS__)
#define SCRIPT_INFO(FMT, ...)		SCRIPT_LOG(Mona::Logger::PRIO_INFO,__FILE__,__LINE__,FMT, ## __VA_ARGS__)
#define SCRIPT_DEBUG(FMT, ...)		SCRIPT_LOG(Mona::Logger::PRIO_DEBUG,__FILE__,__LINE__,FMT, ## __VA_ARGS__)
#define SCRIPT_TRACE(FMT, ...)		SCRIPT_LOG(Mona::Logger::PRIO_TRACE,__FILE__,__LINE__,FMT, ## __VA_ARGS__)

#define SCRIPT_CALLBACK(TYPE,LUATYPE,OBJ)						{int __args=1;lua_State* __pState = pState; bool __destructor=false; bool __thisIsConst=false; TYPE* pObj = Script::ToObject<TYPE,LUATYPE>(__pState,__thisIsConst,true);if(!pObj) return 0; TYPE& OBJ = *pObj;int __results=lua_gettop(__pState);
#define SCRIPT_DESTRUCTOR_CALLBACK(TYPE,LUATYPE,OBJ)			{int __args=1;lua_State* __pState = pState; bool __destructor=true; TYPE* pObj = Script::DestructorCallback<TYPE,LUATYPE>(__pState);if(!pObj) return 0;TYPE& OBJ = *pObj;int __results=lua_gettop(__pState);

#define SCRIPT_CALLBACK_NOTCONST_CHECK							if(__thisIsConst) {SCRIPT_ERROR("const object can't call this method") return 0;}

#define SCRIPT_CALLBACK_RETURN									__results= lua_gettop(__pState)-__results;if(__destructor && __results>0) SCRIPT_WARN("Destructor callback should return 0 arguments") return (__results>=0 ? __results : 0);}



#define SCRIPT_BEGIN(STATE)										if(lua_State* __pState = STATE) { const char* __error=NULL;

#define SCRIPT_CREATE_PERSISTENT_OBJECT(TYPE,LUATYPE,OBJ)		Script::WritePersistentObject<TYPE,LUATYPE>(__pState,OBJ);lua_pop(__pState,1);
#define SCRIPT_ADD_DESTRUCTOR(DESTRUCTOR)						Script::AddObjectDestructor(__pState,DESTRUCTOR);

#define SCRIPT_WRITE_PERSISTENT_OBJECT(TYPE,LUATYPE,OBJ)		Script::WritePersistentObject<TYPE,LUATYPE>(__pState,OBJ);
#define SCRIPT_WRITE_OBJECT(TYPE,LUATYPE,OBJ)					Script::WriteObject<TYPE,LUATYPE>(__pState,OBJ);
#define SCRIPT_WRITE_DATA(READER,COUNT)							Script::WriteData(__pState,READER,COUNT);
#define SCRIPT_WRITE_STRING(VALUE)								lua_pushstring(__pState,VALUE);
#define SCRIPT_WRITE_BINARY(VALUE,SIZE)							lua_pushlstring(__pState,(const char*)VALUE,SIZE);
#define SCRIPT_WRITE_BOOL(VALUE)								lua_pushboolean(__pState,VALUE);
#define SCRIPT_WRITE_FUNCTION(VALUE)							lua_pushcfunction(__pState,VALUE);
#define SCRIPT_WRITE_NUMBER(VALUE)								lua_pushnumber(__pState,(lua_Number)VALUE);
#define SCRIPT_WRITE_INT(VALUE)									lua_pushinteger(__pState,(lua_Integer)VALUE);
#define SCRIPT_WRITE_NIL										lua_pushnil(__pState);

#define SCRIPT_LAST_ERROR										__error

#define SCRIPT_MEMBER_FUNCTION_BEGIN(TYPE,LUATYPE,OBJ,MEMBER)	{ if(lua_getmetatable(__pState,LUA_GLOBALSINDEX)!=0) { lua_getfield(__pState,-1,"__pointers");if(!lua_isnil(__pState,-1)) {lua_replace(__pState,-2);std::string __id;Script::GetObjectID<TYPE,LUATYPE>(OBJ,__id);lua_getfield(__pState,-1,__id.c_str());if(!lua_isnil(__pState,-1)) {lua_getfield(__pState,-1,MEMBER);lua_replace(__pState,-3);}}} else {lua_pushnil(__pState);lua_pushnil(__pState);}if(!lua_isfunction(__pState,-2))lua_pop(__pState,2);else {int __top=lua_gettop(__pState)-1;std::string __name = #TYPE;__name += ".";__name += MEMBER;
#define SCRIPT_FUNCTION_BEGIN(NAME)								{ bool __env=false; if(lua_getmetatable(__pState,LUA_GLOBALSINDEX)!=0) { lua_getfield(__pState,-1,"//env"); lua_replace(__pState,-2); if(!lua_isnil(__pState,-1)) { lua_getfield(__pState,-1,NAME); __env=true;} } else lua_getglobal(__pState,NAME); if(!lua_isfunction(__pState,-1)) lua_pop(__pState,__env ? 2 : 1); else { if(__env) { lua_pushvalue(__pState,-2); lua_setfenv(__pState,-2); lua_replace(__pState,-2); }	int __top=lua_gettop(__pState); string __name = NAME;
#define SCRIPT_FUNCTION_CALL_WITHOUT_LOG						Service::StartVolatileObjectsRecording(__pState);if(lua_pcall(__pState,lua_gettop(__pState)-__top,LUA_MULTRET,0)!=0) { __error = lua_tostring(__pState,-1);} else {--__top;int __results=lua_gettop(__pState);int __args=__top;
#define SCRIPT_FUNCTION_CALL									Service::StartVolatileObjectsRecording(__pState);if(lua_pcall(__pState,lua_gettop(__pState)-__top,LUA_MULTRET,0)!=0) { __error = lua_tostring(__pState,-1);SCRIPT_ERROR("%s",Script::LastError(__pState))} else {--__top;int __results=lua_gettop(__pState);int __args=__top;
#define SCRIPT_FUNCTION_NULL_CALL								{ lua_pop(__pState,lua_gettop(__pState)-__top+1);--__top;int __results=lua_gettop(__pState);int __args=__top;
#define SCRIPT_FUNCTION_END										Service::StopVolatileObjectsRecording(__pState);lua_pop(__pState,__results-__top);__args = __results-__args; if(__args>0) SCRIPT_WARN("%d arguments not required on '%s' results",__args,__name.c_str()) else if(__args<0) SCRIPT_WARN("%d missing arguments on '%s' results",-__args,__name.c_str()) } } }

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
	static void			AddObjectDestructor(lua_State* pState,lua_CFunction destructor);
	
	template<class Type,class LUAType>
	static void GetObjectID(const Type& object,std::string& id) {
		LUAType::ID(id);
		if(id.empty()) {
			id.assign(LUAType::Name);
			id.append(Poco::NumberFormatter::format((void*)&object));
		}
	}

	template<class Type,class LUAType>
	static void ClearPersistentObject(lua_State *pState,const Type& object) {
		if(lua_getmetatable(pState,LUA_GLOBALSINDEX)==0) {
			SCRIPT_BEGIN(pState)
				SCRIPT_WARN("Persistent object clearing impossible without a global table with metatable")
			SCRIPT_END
			return;
		}

		std::string id;
		GetObjectID<Type,LUAType>(object,id);
		const char* idc = id.c_str();
		lua_getfield(pState,-1,idc);
		if(!lua_isnil(pState,-1)) {

			if(lua_getmetatable(pState,-1)!=0) {
				// erase __gcThis in metatable
				lua_getfield(pState,-1,"__gcThis");
				if(!lua_isnil(pState,-1)) {
					lua_pushnil(pState);
					lua_setmetatable(pState,-2);
				}
				lua_pop(pState,2);
			}

			// erase metatable
			lua_pushnil(pState);
			lua_setmetatable(pState,-2);

			// erase pointer
			lua_pushnil(pState);
			lua_setfield(pState,-3,idc);
		}
		lua_pop(pState,2);
	}
	

	template<class Type,class LUAType>
	static void WriteObject(lua_State *pState,Type& object) {
		WriteObject<Type,LUAType>(pState,(const Type&)object);
		// add var
		lua_getmetatable(pState,-1);
		lua_pushnumber(pState,1);
		lua_setfield(pState,-2,"__var");
		lua_pop(pState,1);
	}

	template<class Type,class LUAType>
	static void WriteObject(lua_State *pState,const Type& object) {
		if(lua_getmetatable(pState,LUA_GLOBALSINDEX)==0) {
			SCRIPT_BEGIN(pState)
				SCRIPT_ERROR("Object writing impossible without a global table with metatable")
			SCRIPT_END
			return;
		}
		CreateObject<Type,LUAType>(pState,object);
		lua_replace(pState,-2);
	}

	template<class Type,class LUAType>
	static void WritePersistentObject(lua_State *pState,Type& object) {
		WritePersistentObject<Type,LUAType>(pState,(const Type&)object);
		// add __var
		lua_getmetatable(pState,-1);
		lua_pushnumber(pState,1);
		lua_setfield(pState,-2,"__var");
		lua_pop(pState,1);
	}

	template<class Type,class LUAType>
	static void WritePersistentObject(lua_State *pState,const Type& object) {
		if(lua_getmetatable(pState,LUA_GLOBALSINDEX)==0) {
			SCRIPT_BEGIN(pState)
				SCRIPT_ERROR("Persistent object writing impossible without a global table with metatable")
			SCRIPT_END
			return;
		}
		std::string id;
		GetObjectID<Type,LUAType>(object,id);
		const char* idc = id.c_str();
		lua_getfield(pState,-1,idc);
		if(lua_isnil(pState,-1)) {
			lua_pop(pState,1);
			CreateObject<Type,LUAType>(pState,object);
			lua_pushvalue(pState,-1);
			lua_setfield(pState,-3,idc);
		}
		lua_replace(pState,-2);

		lua_getmetatable(pState,-1);
		// remove __var
		lua_pushnil(pState);
		lua_setfield(pState,-2,"__var");
		// set running
		lua_pushnumber(pState,1);
		lua_setfield(pState,-2,"//running");
		lua_pop(pState,1);
	}

	template<class Type,class LUAType>
	static Type* DestructorCallback(lua_State *pState) {
		if(lua_gettop(pState)==0 || !lua_isuserdata(pState,1)) {
			SCRIPT_BEGIN(pState)
				SCRIPT_ERROR("Bad 'this' argument, bad destructor declaration")
			SCRIPT_END
			return NULL;
		}

		bool isConst;
		return ToObject<Type,LUAType>(pState,isConst,true);
	}

	template<class Type,class LUAType>
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

		// __type is correct type?
		lua_getfield(pState,-1,"__type");
		if(!lua_isstring(pState,-1) || strcmp(lua_tostring(pState,-1),LUAType::Name)!=0) {
			lua_pop(pState,2);
			if(!callback) return NULL;
			SCRIPT_BEGIN(pState)
				SCRIPT_ERROR("bad cast of 'this' argument, call method with ':' colon operator")
			SCRIPT_END
			return NULL;
		}
		lua_pop(pState,1);

		// __this is user data?
		lua_getfield(pState,-1,"__this");
		if(!lua_islightuserdata(pState,-1)) {
			lua_pop(pState,2);
			if(!callback) return NULL;
			SCRIPT_BEGIN(pState)
				SCRIPT_ERROR("bad 'this' argument (bad type)")
			SCRIPT_END
			return NULL;
		}

		Type* pThis = (Type*) lua_touserdata(pState, -1);
		lua_pop(pState,1);

		// persistent checking to avoid to use a Mona object deleted!
		lua_getfield(pState,-1,"//running");
		if(!lua_isnil(pState,-1) && lua_getmetatable(pState,LUA_GLOBALSINDEX)!=0) {
			std::string id;
			GetObjectID<Type,LUAType>(*pThis,id);
			const char* idc = id.c_str();
			lua_getfield(pState,-1,idc);
			if(lua_isnil(pState,-1)) {
				lua_pop(pState,4);
				return NULL;
			}
			lua_pop(pState,2);
		}
		lua_pop(pState,1);

		// isConst?
		lua_getfield(pState,-1,"__var");
		if(lua_isnil(pState,-1))
			isConst=true;

		lua_pop(pState,2);

		return pThis;
	}

	static lua_Debug	LuaDebug;

private:
	static const char* ToString(lua_State* pState,std::string& out);
	static void	ReadData(lua_State *pState,Mona::DataWriter& writer,Mona::UInt32 count,std::map<Mona::UInt64,Mona::UInt32>& references);
	static void WriteData(lua_State *pState,Mona::DataReader::Type type,Mona::DataReader& reader);


	template<class Type,class LUAType>
	static void CreateObject(lua_State *pState,const Type& object) {
		// here glabal metatable is on the top

		// Create table to represent our object
		lua_newtable(pState); 

		// metatable
		lua_newtable(pState); 
	
		// __this
		lua_pushlightuserdata(pState,(void*)&object);
		lua_setfield(pState,-2,"__this");

		// type
		lua_pushstring(pState,LUAType::Name);
		lua_setfield(pState, -2, "__type");

		// call
		lua_pushcfunction(pState,&Script::Call<LUAType>);
		lua_setfield(pState,-2,"__call");

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

		// set in __pointers of global metatable
		lua_getfield(pState,-2,"__pointers");
		if(lua_isnil(pState,-1)) {
			lua_pop(pState,1);
			lua_newtable(pState);
			lua_pushvalue(pState,-1);
			lua_newtable(pState);
			lua_pushstring(pState,"v");
			lua_setfield(pState,-2,"__mode");
			lua_setmetatable(pState,-2);
			lua_setfield(pState,-4,"__pointers");
		}
		
		// record the pointer
		std::string id;
		GetObjectID<Type,LUAType>(object,id);
		lua_pushvalue(pState,-2);
		lua_setfield(pState,-2,id.c_str());
		lua_pop(pState,1);
	}

	template<class LUAType>
	static int Call(lua_State* pState) {
		lua_pushstring(pState,"(");
		lua_insert(pState,2);
		int result = LUAType::Get(pState);
		lua_remove(pState,2);
		return result;
	}

	static int Error(lua_State* pState);
	static int Warn(lua_State* pState);
	static int Note(lua_State* pState);
	static int Info(lua_State* pState);
	static int Debug(lua_State* pState);
	static int Trace(lua_State* pState);

	static int			Panic(lua_State *pState);
	static int			EnvironmentIndex(lua_State *pState);
};
