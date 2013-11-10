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

#include "LUAClient.h"
#include "Mona/Util.h"
#include "LUAWriter.h"


using namespace std;
using namespace Mona;

const char*		LUAClient::Name="Mona::Client";

void LUAClient::Clear(lua_State* pState,const Client& client){
	Script::ClearPersistentObject<Writer,LUAWriter>(pState,((Client&)client).writer());
	Script::ClearPersistentObject<Client,LUAClient>(pState,client);
}

int LUAClient::Get(lua_State *pState) {
	SCRIPT_CALLBACK(Client,LUAClient,client)
		string name = SCRIPT_READ_STRING("");
		if(name=="writer") {
			SCRIPT_CALLBACK_NOTCONST_CHECK
			SCRIPT_WRITE_PERSISTENT_OBJECT(Writer,LUAWriter,client.writer())
		} else if(name=="id") {
			string hex;
			SCRIPT_WRITE_STRING(Util::FormatHex(client.id, ID_SIZE, hex).c_str())
		} else if(name=="rawId") {
			SCRIPT_WRITE_BINARY(client.id,ID_SIZE);
		} else if(name=="path") {
			SCRIPT_WRITE_STRING(client.path.c_str())
		} else if(name=="address") {
			SCRIPT_WRITE_STRING(client.address.toString().c_str())
		} else if(name=="ping") {
			SCRIPT_WRITE_NUMBER(client.ping)
		} else {
			string value;
			if(client.getString(name,value))
				SCRIPT_WRITE_STRING(value.c_str())
		}
	SCRIPT_CALLBACK_RETURN
}

int LUAClient::Set(lua_State *pState) {
	SCRIPT_CALLBACK(Client,LUAClient,client)
		string name = SCRIPT_READ_STRING("");
		if(name=="name") {
			const char* newName = lua_tostring(pState,-1);
			if(!newName)
				SCRIPT_ERROR("Invalid name value")
			else if(!client.setName(newName))
			SCRIPT_ERROR("A client has already the '", newName, "' name")
		} else
			lua_rawset(pState,1); // consumes key and value
	SCRIPT_CALLBACK_RETURN
}




