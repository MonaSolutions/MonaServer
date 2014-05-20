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
	lua_setfield(pState, -2,"|id");
	lua_pop(pState, 1);
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
	UInt32 size = lua_objlen(pState, 2);
	const char* id = lua_tostring(pState, 2);
	if (size == ID_SIZE)
		pClient = pInvoker->clients(id);
	else if (size == (ID_SIZE << 1)) {
		string temp((const char*)id,size);
		pClient = pInvoker->clients((const UInt8*)Util::UnformatHex(temp).c_str());
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

int LUAClient::SetCookie(lua_State *pState) {
	SCRIPT_CALLBACK(Client,client)
 		const char* key = SCRIPT_READ_STRING(NULL);
		const char* value = SCRIPT_READ_STRING(NULL);
		Int32 expiresOffset = SCRIPT_READ_INT(0);
		const char* path = SCRIPT_READ_STRING(NULL);
		const char* domain = SCRIPT_READ_STRING(NULL);
		bool secure = SCRIPT_READ_BOOL(false);
		bool httpOnly = SCRIPT_READ_BOOL(false);

		if (!key)
			SCRIPT_ERROR("cookie's key argument missing")
		else if (!value)
			SCRIPT_ERROR("cookie's value argument missing")
		else {
			string cookieKey, cookieValue;
			String::Append(cookieKey, "cookies.", key);
			String::Format(cookieValue, key, "=", value);

			// Expiration Date in RFC 1123 Format
			if (expiresOffset != 0) {
				Date expiration(Date::Type::GMT);
				expiration += expiresOffset*1000; // Now + signed offset
				string dateExpiration;
				String::Append(cookieValue, "; Expires=", expiration.toString(Date::RFC1123_FORMAT, dateExpiration));
			}

			if (path) String::Append(cookieValue, "; Path=", path);
			if (domain) String::Append(cookieValue, "; Domain=", domain);
			if (secure) String::Append(cookieValue, "; Secure");
			if (httpOnly) String::Append(cookieValue, "; HttpOnly");

			((Parameters&)client.properties()).setString(cookieKey, cookieValue);
		}
	SCRIPT_CALLBACK_RETURN
}

int LUAClient::Get(lua_State *pState) {
	SCRIPT_CALLBACK(Client,client)
		const char* name = SCRIPT_READ_STRING(NULL);
		if (name) {
			if(strcmp(name,"writer")==0) {
				SCRIPT_CALLBACK_NOTCONST_CHECK
				SCRIPT_ADD_OBJECT(Writer,LUAWriter,client.writer())
			} else if(strcmp(name,"id")==0) {
				if (lua_getmetatable(pState, 1)) {
					lua_getfield(pState, -1, "|id");
					lua_replace(pState, -2);
				}
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
			} else if (strcmp(name, "setCookie") == 0) {
				SCRIPT_WRITE_FUNCTION(&LUAClient::SetCookie)
			} else if (strcmp(name,"properties")==0) {
				Script::Collection(pState, 1, "properties");
			} else if (strcmp(name,"parameters")==0 || client.protocol==name) {
				Script::Collection(pState, 1, name);
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




