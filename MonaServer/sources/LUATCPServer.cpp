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

#include "LUATCPServer.h"
#include "LUATCPClient.h"
#include "Service.h"

using namespace std;
using namespace Mona;


const char*		LUATCPServer::Name="LUATCPServer";

LUATCPServer::LUATCPServer(const SocketManager& manager,lua_State* pState) : _pState(pState),TCPServer(manager) {
}

LUATCPServer::~LUATCPServer() {
}

void LUATCPServer::onError(const string& error) {
	WARN("LUATCPServer, ", error);
}

void LUATCPServer::onConnectionRequest(Exception& ex) {
	LUATCPClient* pClient = acceptClient<LUATCPClient>(ex, manager, _pState);
	if (!pClient)
		return;
	SCRIPT_BEGIN(_pState)
		SCRIPT_MEMBER_FUNCTION_BEGIN(LUATCPServer,LUATCPServer,*this,"onConnection")	
			SCRIPT_WRITE_OBJECT(LUATCPClient, LUATCPClient, *pClient)
			SCRIPT_ADD_DESTRUCTOR(&LUATCPClient::Destroy);
			SCRIPT_FUNCTION_CALL
		SCRIPT_FUNCTION_END
	SCRIPT_END
}

int	LUATCPServer::Destroy(lua_State* pState) {
	SCRIPT_DESTRUCTOR_CALLBACK(LUATCPServer,LUATCPServer,server)
		delete &server;
	SCRIPT_CALLBACK_RETURN
}

int	LUATCPServer::Start(lua_State* pState) {
	SCRIPT_CALLBACK(LUATCPServer, LUATCPServer, server)
		string host("0.0.0.0");
		if (SCRIPT_NEXT_TYPE == LUA_TSTRING)
			host = SCRIPT_READ_STRING("0.0.0.0");
		UInt16 port = SCRIPT_READ_UINT(0);
		Exception ex;
		SocketAddress address;
		if (port == 0)
			address.set(ex, host);
		else
			address.set(ex, host, port);
		if (!ex)
			server.start(ex, address);
		if (ex)
			SCRIPT_WRITE_STRING(ex.error().c_str())
	SCRIPT_CALLBACK_RETURN
}

int	LUATCPServer::Stop(lua_State* pState) {
	SCRIPT_CALLBACK(LUATCPServer,LUATCPServer,server)
		server.stop();
	SCRIPT_CALLBACK_RETURN
}

int LUATCPServer::Get(lua_State* pState) {
	SCRIPT_CALLBACK(LUATCPServer,LUATCPServer,server)
		string name = SCRIPT_READ_STRING("");
		if(name=="start") {
			SCRIPT_WRITE_FUNCTION(&LUATCPServer::Start)
		} else if(name=="stop") {
			SCRIPT_WRITE_FUNCTION(&LUATCPServer::Stop)
		} else if(name=="address") {
			SCRIPT_WRITE_STRING(server.address().toString().c_str())
		} else if(name=="running")
			SCRIPT_WRITE_BOOL(server.running())
	SCRIPT_CALLBACK_RETURN
}

int LUATCPServer::Set(lua_State* pState) {
	SCRIPT_CALLBACK(LUATCPServer,LUATCPServer,server)
		string name = SCRIPT_READ_STRING("");
		lua_rawset(pState,1); // consumes key and value
	SCRIPT_CALLBACK_RETURN
}
