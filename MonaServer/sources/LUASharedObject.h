#pragma once
#include "SharedObject.h"
#include "Script.h"
class LUASharedObject
{
public:
	static int Item(lua_State *pState);

	static void Init(lua_State *pState, Mona::SharedObject& so);
	static void Clear(lua_State* pState, Mona::SharedObject& so);
	static int  Get(lua_State* pState);
	static int  Set(lua_State* pState);
	static int SetProperty(lua_State* pState);
	static int Flush(lua_State* pState);
	static void AMFToLUA(lua_State* pState, Mona::AMFObject* object);
};

