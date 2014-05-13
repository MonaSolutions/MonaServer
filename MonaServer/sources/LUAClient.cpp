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
#include "Mona/Invoker.h"
#include "Mona/Util.h"
#include "LUAWriter.h"
#include "LUAQualityOfService.h"


using namespace std;
using namespace Mona;


void LUAClient::GetID(lua_State *pState, const Client& client) {
	string key;
	if (!client.properties().getString("|id", key))
		((Peer&)client).properties().setString("|id", Util::FormatHex(client.id, ID_SIZE, key));
	lua_pushstring(pState, key.c_str());
}

void LUAClient::Clear(lua_State* pState,const Client& client){
	Script::ClearObject<Writer, LUAWriter>(pState, ((Client&)client).writer());
	Script::ClearObject<QualityOfService, LUAQualityOfService>(pState, ((Client&)client).writer().qos());
}

int LUAClient::Item(lua_State *pState) {
	// 1 => clients table
	// 2 => parameter
	if (!lua_isstring(pState, 2))
		return 0;
	Invoker* pInvoker = Script::GetCollector<Invoker>(pState,1);
	if (!pInvoker)
		return 0;
	Client* pClient(NULL);
	UInt32 size = lua_objlen(pState, 2);
	const char* id = lua_tostring(pState, 2);
	if (size == ID_SIZE)
		pClient = pInvoker->clients(id);
	else if (size == (ID_SIZE << 1)) {
		pInvoker->buffer.assign((const char*)id,size);
		pClient = pInvoker->clients((const UInt8*)Util::UnformatHex(pInvoker->buffer).c_str());
	}

	if (!pClient) {
		string name(id, size);
		pClient = pInvoker->clients(name); // try by name!
	}
	SCRIPT_BEGIN(pState)
		if (pClient)
			SCRIPT_ADD_OBJECT(Client, LUAClient,*pClient)
	SCRIPT_END
	return pClient ? 1 : 0;
}

int LUAClient::Get(lua_State *pState) {
	SCRIPT_CALLBACK(Client,client)
		const char* name = SCRIPT_READ_STRING(NULL);
		if (name) {
			if(strcmp(name,"writer")==0) {
				SCRIPT_CALLBACK_NOTCONST_CHECK
				SCRIPT_ADD_OBJECT(Writer,LUAWriter,client.writer())
			} else if(strcmp(name,"id")==0) {
				LUAClient::GetID(pState, client);
			} else if(strcmp(name,"name")==0) {
				SCRIPT_WRITE_STRING(client.name.c_str())
			} else if(strcmp(name,"rawId")==0) {
				SCRIPT_WRITE_BINARY(client.id,ID_SIZE);
			} else if(strcmp(name,"path")==0) {
				SCRIPT_WRITE_STRING(client.path.c_str())
			} else if(strcmp(name,"query")==0) {
				SCRIPT_WRITE_STRING(client.query.c_str())
			} else if(strcmp(name,"address")==0) {
				SCRIPT_WRITE_STRING(client.address.toString().c_str())
			} else if(strcmp(name,"ping")==0) {
				SCRIPT_WRITE_NUMBER(client.ping)
			} else if(strcmp(name,"protocol")==0) {
				SCRIPT_WRITE_STRING(client.protocol.c_str())
			} else if (strcmp(name,"properties")==0) {
				if (Script::Collection(pState, 1, "properties", client.properties().count())) {
					for (auto& it : ((Peer&)client).properties())
						Script::SetProperty(pState, it.first, it.second);
				}
			} else if (strcmp(name,"parameters")==0 || client.protocol==name) {
				if (Script::Collection(pState, 1, name, client.parameters().count())) {
					Parameters::ForEach forEach([pState](const string& key, const string& value) {
						Script::SetProperty(pState, key, value);
					});
					client.parameters().iterate(forEach);
				}
			} else {
				string value;
				if (client.properties().getString(name, value))
					Script::PushValue(pState,value);
			}
		}
	SCRIPT_CALLBACK_RETURN
}

int LUAClient::Set(lua_State *pState) {
	SCRIPT_CALLBACK(Client,client)
		lua_rawset(pState,1); // consumes key and value
	SCRIPT_CALLBACK_RETURN
}




