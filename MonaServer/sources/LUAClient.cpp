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


void LUAClient::GetKey(lua_State *pState, const Client& client) {
	string key;
	if (client.getString("name", key))
		lua_pushstring(pState, key.c_str());
	else if (!client.getString("__hexID", key))
		((Client&)client).setString("__hexID", Util::FormatHex(client.id, ID_SIZE, key));
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
	else if (size == (ID_SIZE * 2))
		pClient = pInvoker->clients(Util::UnformatHex((UInt8*)id, size));
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
		string name = SCRIPT_READ_STRING("");
		if(name=="writer") {
			SCRIPT_CALLBACK_NOTCONST_CHECK
			SCRIPT_ADD_OBJECT(Writer,LUAWriter,client.writer())
		} else if(name=="id") {
			LUAClient::GetKey(pState, client);
		} else if(name=="rawId") {
			SCRIPT_WRITE_BINARY(client.id,ID_SIZE);
		} else if(name=="path") {
			SCRIPT_WRITE_STRING(client.path.c_str())
		} else if(name=="address") {
			SCRIPT_WRITE_STRING(client.address.toString().c_str())
		} else if(name=="ping") {
			SCRIPT_WRITE_NUMBER(client.ping)
		} else if (name == "parameters") {
			if (Script::Collection(pState, -1, "parameters", client.count())) {
				for (auto& it : client) {
					lua_pushstring(pState, it.first.c_str());
					if (String::ICompare(it.second, "false") == 0 || String::ICompare(it.second, "nil") == 0)
						lua_pushboolean(pState, 0);
					else
						lua_pushlstring(pState, it.second.c_str(), it.second.size());
					lua_rawset(pState, -3); // rawset cause NewIndexProhibited
				}
			}
		} else {
			string value;
			if(client.getString(name,value))
				SCRIPT_WRITE_STRING(value.c_str())
		}
	SCRIPT_CALLBACK_RETURN
}

int LUAClient::Set(lua_State *pState) {
	SCRIPT_CALLBACK(Client,client)
		string name = SCRIPT_READ_STRING("");
		if (name == "name") {
			const char* newName = SCRIPT_READ_STRING(NULL);
			if (!newName)
				SCRIPT_ERROR("Invalid name value")
			else if (!client.setName(newName))
				SCRIPT_ERROR("A client has already the '", newName, "' name")
		} else if(name=="__timesBeforeTurn") {
			client.setNumber("__timesBeforeTurn", SCRIPT_READ_UINT(0));
		} else
			lua_rawset(pState,1); // consumes key and value
	SCRIPT_CALLBACK_RETURN
}




