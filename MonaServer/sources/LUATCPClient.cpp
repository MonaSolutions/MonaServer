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

#include "LUATCPClient.h"
#include "Service.h"

using namespace std;
using namespace Mona;
using namespace Poco;
using namespace Poco::Net;


const char*		LUATCPClient::Name="LUATCPClient";

LUATCPClient::LUATCPClient(const StreamSocket& socket,const SocketManager& manager,lua_State* pState) : _pState(pState),TCPClient(socket,manager) {
}

LUATCPClient::LUATCPClient(const SocketManager& manager,lua_State* pState) : _pState(pState),TCPClient(manager) {
}

LUATCPClient::~LUATCPClient() {
}

Mona::UInt32 LUATCPClient::onReception(const Mona::UInt8* data,Mona::UInt32 size){
	Mona::UInt32 rest=0;
	SCRIPT_BEGIN(_pState)
		SCRIPT_MEMBER_FUNCTION_BEGIN(LUATCPClient,LUATCPClient,*this,"onReception")
			SCRIPT_WRITE_BINARY(data,size)
			SCRIPT_FUNCTION_CALL
			rest = SCRIPT_READ_UINT(0);
		SCRIPT_FUNCTION_END
	SCRIPT_END
	return rest;
}

void LUATCPClient::onDisconnection(){
	SCRIPT_BEGIN(_pState)
		SCRIPT_MEMBER_FUNCTION_BEGIN(LUATCPClient,LUATCPClient,*this,"onDisconnection")
			SCRIPT_FUNCTION_CALL
		SCRIPT_FUNCTION_END
	SCRIPT_END
}

int	LUATCPClient::Destroy(lua_State* pState) {
	SCRIPT_DESTRUCTOR_CALLBACK(LUATCPClient,LUATCPClient,client)
		delete &client;
	SCRIPT_CALLBACK_RETURN
}

int	LUATCPClient::Connect(lua_State* pState) {
	SCRIPT_CALLBACK(LUATCPClient,LUATCPClient,client)
		string host = SCRIPT_READ_STRING("");
		Mona::UInt16 port = SCRIPT_READ_UINT(0);
		try {
			SocketAddress address(host,port);
			client.connect(address);
			if(client.error())
				SCRIPT_WRITE_STRING(client.error())
		} catch(Exception& ex) {
			SCRIPT_WRITE_STRING(format("Understandable TCPClient address, %s",ex.displayText()).c_str())
		}
	SCRIPT_CALLBACK_RETURN
}

int	LUATCPClient::Disconnect(lua_State* pState) {
	SCRIPT_CALLBACK(LUATCPClient,LUATCPClient,client)
		client.disconnect();
	SCRIPT_CALLBACK_RETURN
}


int	LUATCPClient::Send(lua_State* pState) {
	SCRIPT_CALLBACK(LUATCPClient,LUATCPClient,client)
		SCRIPT_READ_BINARY(data,size)
		if(!client.connected()) {
			SCRIPT_ERROR("TCPClient not connected");
		} else {
			client.send(data,size);
			if(client.error())
				SCRIPT_WRITE_STRING(client.error())
		}
	SCRIPT_CALLBACK_RETURN
}

int LUATCPClient::Get(lua_State* pState) {
	SCRIPT_CALLBACK(LUATCPClient,LUATCPClient,client)
		string name = SCRIPT_READ_STRING("");
		if(name=="connect") {
			SCRIPT_WRITE_FUNCTION(&LUATCPClient::Connect)
		} else if(name=="disconnect") {
			SCRIPT_WRITE_FUNCTION(&LUATCPClient::Disconnect)
		} else if(name=="send") {
			SCRIPT_WRITE_FUNCTION(&LUATCPClient::Send)
		} else if(name=="address") {
			if(client.connected())
				SCRIPT_WRITE_STRING(client.address().toString().c_str())
			else
				SCRIPT_WRITE_NIL
		} else if(name=="peerAddress") {
			if(client.connected())
				SCRIPT_WRITE_STRING(client.peerAddress().toString().c_str())
			else
				SCRIPT_WRITE_NIL
		} else if(name=="error") {
			if(client.error())
				SCRIPT_WRITE_STRING(client.error())
			else
				SCRIPT_WRITE_NIL
		} else if(name=="connected")
			SCRIPT_WRITE_BOOL(client.connected())
	SCRIPT_CALLBACK_RETURN
}

int LUATCPClient::Set(lua_State* pState) {
	SCRIPT_CALLBACK(LUATCPClient,LUATCPClient,client)
		string name = SCRIPT_READ_STRING("");
		lua_rawset(pState,1); // consumes key and value
	SCRIPT_CALLBACK_RETURN
}
