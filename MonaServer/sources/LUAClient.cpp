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
#include "Mona/ArrayReader.h"
#include "Mona/ArrayWriter.h"
#include "Mona/Util.h"
#include "LUAWriter.h"
#include "LUAQualityOfService.h"


using namespace std;
using namespace Mona;

int LUAClient::LUAProperties::Item(lua_State *pState) {
	// 1 => properties table
	// ... => parameters
	if (!lua_isstring(pState, 2))
		return 0;
	Client* pClient = Script::GetCollector<Client>(pState,1);
	if (!pClient)
		return 0;

	vector<string> items;
	ArrayWriter<vector<string>> writer(items);

	Script::ReadData(pState,writer,lua_gettop(pState)-1).endWrite();
	pClient->properties(items);
	ArrayReader<vector<string>> reader(items);
	Script::WriteData(pState,reader);
	return items.size();
}

void LUAClient::Init(lua_State* pState, Client& client) {
	Script::InitCollectionParameters<Client,LUAClient::LUAProperties>(pState,client,"properties",((Peer&)client).properties(),&client);
	Script::InitCollectionParameters(pState,client,"parameters",client.parameters());

	lua_getmetatable(pState, -1);
	string hex;
	lua_pushstring(pState, Mona::Util::FormatHex(client.id, ID_SIZE, hex).c_str());
	lua_pushvalue(pState, -1);
	lua_setfield(pState, -3,"|id");
	lua_replace(pState, -2);
	lua_setfield(pState, -2, "id");
}


void LUAClient::Clear(lua_State* pState,const Client& client){
	Script::ClearCollectionParameters(pState,"properties",((Peer&)client).properties());
	Script::ClearCollectionParameters(pState,"parameters",client.parameters());

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
	size_t size = lua_objlen(pState, 2);
	const UInt8* id((const UInt8*)lua_tolstring(pState, 2,&size));
	if (size == ID_SIZE)
		pClient = pInvoker->clients(id);
	else if (size == (ID_SIZE << 1)) {
		string temp((const char*)id,size);
		pClient = pInvoker->clients((const UInt8*)Util::UnformatHex(temp).c_str());
	}

	if (!pClient)
		pClient = pInvoker->clients((const char*)id); // try by name!

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
				SCRIPT_CALLBACK_FIX_INDEX(name)
			} else if(strcmp(name,"id")==0) {
				lua_getmetatable(pState, 1);
				lua_getfield(pState, -1, "|id");
				lua_replace(pState, -2);
				SCRIPT_CALLBACK_FIX_INDEX(name)
			} else if(strcmp(name,"name")==0) {
				SCRIPT_WRITE_STRING(client.name.c_str())
				SCRIPT_CALLBACK_FIX_INDEX(name)
			} else if(strcmp(name,"rawId")==0) {
				SCRIPT_WRITE_BINARY(client.id,ID_SIZE);
				SCRIPT_CALLBACK_FIX_INDEX(name)
			} else if(strcmp(name,"path")==0) {
				SCRIPT_WRITE_STRING(client.path.c_str())
				SCRIPT_CALLBACK_FIX_INDEX(name)
			} else if(strcmp(name,"query")==0) {
				SCRIPT_WRITE_STRING(client.query.c_str())
				SCRIPT_CALLBACK_FIX_INDEX(name)
			} else if(strcmp(name,"address")==0) {
				SCRIPT_WRITE_STRING(client.address.toString().c_str())
			} else if(strcmp(name,"ping")==0) {
				SCRIPT_WRITE_NUMBER(client.ping)
			} else if(strcmp(name,"protocol")==0) {
				SCRIPT_WRITE_STRING(client.protocol.c_str())
				SCRIPT_CALLBACK_FIX_INDEX(name)
			} else if (strcmp(name,"properties")==0) {
				Script::Collection(pState, 1, "properties");
				SCRIPT_CALLBACK_FIX_INDEX(name)
			} else if (strcmp(name,"parameters")==0 || client.protocol==name) {
				Script::Collection(pState, 1, name);
				SCRIPT_CALLBACK_FIX_INDEX(name)
			} else {
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




