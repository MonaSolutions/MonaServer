/* 
	Copyright 2010 Mona - mathieu.poux[a]gmail.com
 
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

#include "LUAUDPSocket.h"
#include "Service.h"

using namespace std;
using namespace Mona;
using namespace Poco;
using namespace Poco::Net;


const char*		LUAUDPSocket::Name="LUAUDPSocket";


LUAUDPSocket::LUAUDPSocket(const SocketManager& manager,bool allowBroadcast,lua_State* pState) : _pState(pState),UDPSocket(manager,allowBroadcast) {
}

LUAUDPSocket::~LUAUDPSocket() {
}

void LUAUDPSocket::onReception(const Mona::UInt8* data,Mona::UInt32 size,const SocketAddress& address){
	SCRIPT_BEGIN(_pState)
		SCRIPT_MEMBER_FUNCTION_BEGIN(LUAUDPSocket,LUAUDPSocket,*this,"onReception")
			SCRIPT_WRITE_BINARY(data,size)
			SCRIPT_WRITE_STRING(address.toString().c_str())
			SCRIPT_FUNCTION_CALL
		SCRIPT_FUNCTION_END
	SCRIPT_END
}

int	LUAUDPSocket::Destroy(lua_State* pState) {
	SCRIPT_DESTRUCTOR_CALLBACK(LUAUDPSocket,LUAUDPSocket,udp)
		delete &udp;
	SCRIPT_CALLBACK_RETURN
}

int	LUAUDPSocket::Close(lua_State* pState) {
	SCRIPT_CALLBACK(LUAUDPSocket,LUAUDPSocket,udp)
		udp.close();
	SCRIPT_CALLBACK_RETURN
}

int	LUAUDPSocket::Connect(lua_State* pState) {
	SCRIPT_CALLBACK(LUAUDPSocket,LUAUDPSocket,udp)
		const char* address = SCRIPT_READ_STRING(NULL);
		if(address) {
			try {
				SocketAddress addr(address);
				udp.connect(addr);
				if(udp.error())
					SCRIPT_ERROR(udp.error())
			} catch(Poco::Exception& ex) {
				SCRIPT_ERROR("Understandable address, ",ex.displayText())
			}
		} else
			SCRIPT_ERROR("UDPSocket::connect takes a valid address in first argument");
	SCRIPT_CALLBACK_RETURN
}


int	LUAUDPSocket::Bind(lua_State* pState) {
	SCRIPT_CALLBACK(LUAUDPSocket,LUAUDPSocket,udp)
		const char* address = SCRIPT_READ_STRING(NULL);
		if(address) {
			try {
				SocketAddress addr(address);
				if(!udp.bind(addr))
					SCRIPT_WRITE_STRING(udp.error())
			} catch(Poco::Exception& ex) {
				SCRIPT_WRITE_STRING(format("Understandable address, %s",ex.displayText()).c_str())
			}
		} else
			SCRIPT_WRITE_STRING("UDPSocket::bind takes a valid address in first argument");
	SCRIPT_CALLBACK_RETURN
}


int	LUAUDPSocket::Send(lua_State* pState) {
	SCRIPT_CALLBACK(LUAUDPSocket,LUAUDPSocket,udp)
		SCRIPT_READ_BINARY(data,size)
		const char* address = SCRIPT_READ_STRING(NULL);
		try {
			if(address) {
				SocketAddress addr(address);
				udp.send(data,size,addr);
			} else {
				udp.send(data,size);
			}
			if(udp.error())
				SCRIPT_ERROR(udp.error())
		} catch(Poco::Exception& ex) {
			SCRIPT_ERROR("Understandable address, ",ex.displayText())
		}
	SCRIPT_CALLBACK_RETURN
}

int LUAUDPSocket::Get(lua_State* pState) {
	SCRIPT_CALLBACK(LUAUDPSocket,LUAUDPSocket,udp)
		string name = SCRIPT_READ_STRING("");
		if(name=="connect") {
			SCRIPT_WRITE_FUNCTION(&LUAUDPSocket::Connect)
		} else if(name=="close") {
			SCRIPT_WRITE_FUNCTION(&LUAUDPSocket::Close)
		} else if(name=="send") {
			SCRIPT_WRITE_FUNCTION(&LUAUDPSocket::Send)
		} else if(name=="bind") {
			SCRIPT_WRITE_FUNCTION(&LUAUDPSocket::Bind)
		} else if(name=="address") {
			if(!udp.address().host().isWildcard())
				SCRIPT_WRITE_STRING(udp.address().toString().c_str())
			else
				SCRIPT_WRITE_NIL
		} else if(name=="peerAddress") {
			if(!udp.peerAddress().host().isWildcard())
				SCRIPT_WRITE_STRING(udp.peerAddress().toString().c_str())
			else
				SCRIPT_WRITE_NIL
		}
	SCRIPT_CALLBACK_RETURN
}

int LUAUDPSocket::Set(lua_State* pState) {
	SCRIPT_CALLBACK(LUAUDPSocket,LUAUDPSocket,udp)
		string name = SCRIPT_READ_STRING("");
		lua_rawset(pState,1); // consumes key and value
	SCRIPT_CALLBACK_RETURN
}
