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

#include "LUAClient.h"
#include "Mona/Peer.h"
#include "Mona/Util.h"
#include "ScriptReader.h"
#include "ScriptWriter.h"
#include "LUAWriter.h"
#include "LUAQualityOfService.h"
#include "LUASocketAddress.h"


using namespace std;
using namespace Mona;

int LUAClient::Item(lua_State *pState) {
	SCRIPT_CALLBACK(Client,client)
		ScriptReader reader(pState, SCRIPT_READ_AVAILABLE);
		string value("null");
		while (reader.available() && client.OnCallProperties::raise<false>(reader, value)) {
			Script::PushValue(pState, value);
			value.assign("null");
		}
		while (reader.readString(value)) {
			if (client.properties().getString(value, value))
				lua_pushlstring(pState, value.data(), value.size());
			else
				lua_pushnil(pState);
		}
	SCRIPT_CALLBACK_RETURN
}

void LUAClient::Init(lua_State* pState, Client& client) {
	Script::InitCollectionParameters<LUAClient>(pState,client,"properties",((Peer&)client).properties());
	Script::InitCollectionParameters(pState,client,"parameters",client.parameters());

	lua_getmetatable(pState, -1);
	string hex;
	lua_pushstring(pState, Mona::Util::FormatHex(client.id, ID_SIZE, hex).c_str());
	lua_pushvalue(pState, -1);
	lua_setfield(pState, -3,"|id");
	lua_replace(pState, -2);
	lua_setfield(pState, -2, "id");
}


void LUAClient::Clear(lua_State* pState,Client& client){
	Script::ClearCollectionParameters(pState,"properties",((Peer&)client).properties());
	Script::ClearCollectionParameters(pState,"parameters",client.parameters());

	Script::ClearObject<LUAWriter>(pState, client.writer());
	Script::ClearObject<LUASocketAddress>(pState, client.address);
}


int LUAClient::Get(lua_State *pState) {
	SCRIPT_CALLBACK(Client,client)
		const char* name = SCRIPT_READ_STRING(NULL);
		if (name) {
			if(strcmp(name,"writer")==0) {
				SCRIPT_CALLBACK_NOTCONST_CHECK
				Script::AddObject<LUAWriter>(pState,client.writer());
				SCRIPT_FIX_RESULT
			} else if(strcmp(name,"id")==0) {
				lua_getmetatable(pState, 1);
				lua_getfield(pState, -1, "|id");
				lua_replace(pState, -2);
				SCRIPT_FIX_RESULT
			} else if(strcmp(name,"rawId")==0) {
				SCRIPT_WRITE_BINARY(client.id,ID_SIZE);
				SCRIPT_FIX_RESULT
			} else if(strcmp(name,"path")==0) {
				SCRIPT_WRITE_STRING(client.path.c_str())
				SCRIPT_FIX_RESULT
			} else if(strcmp(name,"query")==0) {
				SCRIPT_WRITE_STRING(client.query.c_str()) // can change (HTTP client for example)
			} else if(strcmp(name,"address")==0) {
				Script::AddObject<LUASocketAddress>(pState, client.address); // can change (RTMFP client for example)
			} else if(strcmp(name,"ping")==0) {
				SCRIPT_WRITE_NUMBER(client.ping())  // can change
			} else if(strcmp(name,"lastReceptionTime")==0) {
				SCRIPT_WRITE_NUMBER(client.lastReceptionTime)  // can change
			} else if(strcmp(name,"protocol")==0) {
				SCRIPT_WRITE_STRING(client.protocol.c_str())
				SCRIPT_FIX_RESULT
			} else if (strcmp(name,"properties")==0) {
				Script::Collection(pState, 1, "properties");
				SCRIPT_FIX_RESULT
			} else if (strcmp(name,"parameters")==0 || client.protocol==name) {
				Script::Collection(pState, 1, name);
				SCRIPT_FIX_RESULT
			} else {
				// can change
				Script::Collection(pState,1, "properties");
				lua_getfield(pState, -1, name);
				lua_replace(pState, -2);
			}
		}
	SCRIPT_CALLBACK_RETURN
}

int LUAClient::Set(lua_State *pState) {
	SCRIPT_CALLBACK(Client,client)
		lua_rawset(pState,1); // consumes key and value
	SCRIPT_CALLBACK_RETURN
}




