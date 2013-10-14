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
using namespace Poco;
using namespace Poco::Net;


const char*		LUATCPServer::Name="LUATCPServer";

LUATCPServer::LUATCPServer(const SocketManager& manager,lua_State* pState) : _pState(pState),TCPServer(manager) {
}

LUATCPServer::~LUATCPServer() {
}

void LUATCPServer::clientHandler(StreamSocket& socket){
	SCRIPT_BEGIN(_pState)
		SCRIPT_MEMBER_FUNCTION_BEGIN(LUATCPServer,LUATCPServer,*this,"clientHandler")
			SCRIPT_WRITE_OBJECT(LUATCPClient,LUATCPClient,*(new LUATCPClient(socket,manager,_pState)))
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
	SCRIPT_CALLBACK(LUATCPServer,LUATCPServer,server)
		SCRIPT_WRITE_BOOL(server.start(SCRIPT_READ_UINT(0)))
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
		} else if(name=="port") {
			SCRIPT_WRITE_INT(server.port())
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
