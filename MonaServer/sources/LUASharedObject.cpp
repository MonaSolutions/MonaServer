#include "LUASharedObject.h"
#include "ScriptReader.h"

using namespace Mona;
int LUASharedObject::Item(lua_State* pState)
{
	SCRIPT_CALLBACK(SharedObject,so)

	SCRIPT_CALLBACK_RETURN
}
void LUASharedObject::Init(lua_State *pState, SharedObject& so)
{
	
}
void LUASharedObject::Clear(lua_State* pState, SharedObject& so)
{
	
}
int  LUASharedObject::Get(lua_State* pState)
{
	SCRIPT_CALLBACK(SharedObject, so)
		const char* name = SCRIPT_READ_STRING(NULL);
	if (name) {
		if (strcmp(name, "data") == 0) {
			lua_newtable(pState);
			for (auto& it : so.payloads) {
				AMFToLUA(pState, &it.second);
				lua_setfield(pState, -2, it.first.c_str());
			}
			SCRIPT_CALLBACK_FIX_INDEX
		}else if(strcmp(name, "setProperty") == 0)
		{
			SCRIPT_WRITE_FUNCTION(LUASharedObject::SetProperty)
			SCRIPT_CALLBACK_FIX_INDEX
		}else if(strcmp(name, "flush") == 0)
		{
			SCRIPT_WRITE_FUNCTION(LUASharedObject::Flush)
			SCRIPT_CALLBACK_FIX_INDEX
		}
	}
	SCRIPT_CALLBACK_RETURN
}
int  LUASharedObject::Set(lua_State* pState)
{
	SCRIPT_CALLBACK(SharedObject, so)
		lua_rawset(pState, 1); // consumes key and value
	SCRIPT_CALLBACK_RETURN
}
int LUASharedObject::SetProperty(lua_State* pState)
{
	SCRIPT_CALLBACK(SharedObject, so)
		const char* propertyName(SCRIPT_READ_STRING(NULL));
		AMFObjectWriter writer;
		SCRIPT_READ_NEXT(ScriptReader(pState, SCRIPT_READ_AVAILABLE).read(writer));
		if (writer.amfObject.isNull())so.unSet(propertyName);
		else so.set(propertyName, move(writer.amfObject));
	SCRIPT_CALLBACK_RETURN
}
int LUASharedObject::Flush(lua_State* pState)
{
	SCRIPT_CALLBACK(SharedObject, so)
		so.track();
	SCRIPT_CALLBACK_RETURN
}
void LUASharedObject::AMFToLUA(lua_State* pState, Mona::AMFObject* object)
{
	switch (object->type) {
	case AMF_NULL:
	case AMF_UNDEFINED:
		lua_pushnil(pState);
		return;
	case AMF_NUMBER:
		lua_pushnumber(pState, object->value.num);
		return;
	case AMF_BOOLEAN:
		lua_pushboolean(pState, object->value.b);
		return;
	case AMF_STRING:
		lua_pushstring(pState, object->value.str->c_str());
		return;
	case AMF_MIXED_ARRAY:
		/*	result.write4BE(((*this)["length"]).value.num);
		this->amf0encObject(result);*/
		break;
	case AMF_BEGIN_OBJECT:
	{
		lua_newtable(pState);
		for (auto& item : *object->value.prop) {
			AMFToLUA(pState, &item.second);
			lua_setfield(pState, -2, item.first.c_str());
		}
	}
	break;
	}
}
