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

#include "LUAUDPSocket.h"
#include "Service.h"

using namespace std;
using namespace Mona;


LUAUDPSocket::LUAUDPSocket(const SocketManager& manager,bool allowBroadcast,lua_State* pState) : _pState(pState),UDPSocket(manager,allowBroadcast) {
	
	onError = [this](const Exception& ex) {
		WARN("LUAUDPSocket, ", ex.error());
	};

	onPacket = [this](PoolBuffer& pBuffer, const SocketAddress& address) {
		SCRIPT_BEGIN(_pState)
			SCRIPT_MEMBER_FUNCTION_BEGIN(LUAUDPSocket,*this,"onPacket")
				SCRIPT_WRITE_BINARY(pBuffer->data(),pBuffer->size())
				SCRIPT_WRITE_STRING(address.toString().c_str())
				SCRIPT_FUNCTION_CALL
			SCRIPT_FUNCTION_END
		SCRIPT_END
	};

	OnError::subscribe(onError);
	OnPacket::subscribe(onPacket);
}

LUAUDPSocket::~LUAUDPSocket() {
	OnPacket::unsubscribe(onPacket);
	OnError::unsubscribe(onError);
}


int	LUAUDPSocket::Destroy(lua_State* pState) {
	SCRIPT_DESTRUCTOR_CALLBACK(LUAUDPSocket,udp)
		delete &udp;
	SCRIPT_CALLBACK_RETURN
}

int	LUAUDPSocket::Close(lua_State* pState) {
	SCRIPT_CALLBACK(LUAUDPSocket,udp)
		udp.close();
	SCRIPT_CALLBACK_RETURN
}

int	LUAUDPSocket::Connect(lua_State* pState) {
	SCRIPT_CALLBACK(LUAUDPSocket,udp)
		string host("127.0.0.1");
		if (SCRIPT_NEXT_TYPE == LUA_TSTRING)
			host = SCRIPT_READ_STRING(host);
		UInt16 port = SCRIPT_READ_UINT(0);
		Exception ex;
		SocketAddress address;
		if (port == 0)
			address.set(ex, host);
		else
			address.set(ex, host, port);
		if (!ex)
			udp.connect(ex, address);
		if (ex)
			SCRIPT_WRITE_STRING(ex.error().c_str())
	SCRIPT_CALLBACK_RETURN
}

int	LUAUDPSocket::Disconnect(lua_State* pState) {
	SCRIPT_CALLBACK(LUAUDPSocket,udp)
		udp.disconnect();
	SCRIPT_CALLBACK_RETURN
}



int	LUAUDPSocket::Bind(lua_State* pState) {
	SCRIPT_CALLBACK(LUAUDPSocket,udp)
		string host("0.0.0.0");
		if (SCRIPT_NEXT_TYPE == LUA_TSTRING)
			host = SCRIPT_READ_STRING(host);
		UInt16 port = SCRIPT_READ_UINT(0);
		Exception ex;
		SocketAddress address;
		if (port == 0)
			address.set(ex, host);
		else
			address.set(ex, host, port);
		if (!ex)
			udp.bind(ex, address);
		if (ex)
			SCRIPT_WRITE_STRING(ex.error().c_str())
	SCRIPT_CALLBACK_RETURN
}


int	LUAUDPSocket::Send(lua_State* pState) {
	SCRIPT_CALLBACK(LUAUDPSocket,udp)
		SCRIPT_READ_BINARY(data,size)

		Exception ex;
		string host("127.0.0.1");
		if (SCRIPT_NEXT_TYPE == LUA_TSTRING)
			host = SCRIPT_READ_STRING("127.0.0.1");
		if (SCRIPT_CAN_READ) {
			UInt16 port = SCRIPT_READ_UINT(0);
			SocketAddress address;
			if (port == 0)
				address.set(ex, host);
			else
				address.set(ex, host, port);
			if (!ex)
				udp.send(ex, data,size,address);
		} else
			udp.send(ex,data, size);
		if (ex)
			SCRIPT_WRITE_STRING(ex.error().c_str())
	SCRIPT_CALLBACK_RETURN
}

int LUAUDPSocket::Get(lua_State* pState) {
	SCRIPT_CALLBACK(LUAUDPSocket,udp)
		const char* name = SCRIPT_READ_STRING(NULL);
		if(name) {
			if(strcmp(name,"connect")==0) {
				SCRIPT_WRITE_FUNCTION(LUAUDPSocket::Connect)
				SCRIPT_CALLBACK_FIX_INDEX(name)
			} else if (strcmp(name, "disconnect") == 0) {
				SCRIPT_WRITE_FUNCTION(LUAUDPSocket::Disconnect)
				SCRIPT_CALLBACK_FIX_INDEX(name)
			} else if (strcmp(name, "close") == 0) {
				SCRIPT_WRITE_FUNCTION(LUAUDPSocket::Close)
				SCRIPT_CALLBACK_FIX_INDEX(name)
			} else if (strcmp(name, "send") == 0) {
				SCRIPT_WRITE_FUNCTION(LUAUDPSocket::Send)
				SCRIPT_CALLBACK_FIX_INDEX(name)
			} else if (strcmp(name, "bind") == 0) {
				SCRIPT_WRITE_FUNCTION(LUAUDPSocket::Bind)
				SCRIPT_CALLBACK_FIX_INDEX(name)
			} else if (strcmp(name, "address") == 0) {
				if(!udp.address().host().isWildcard())
					SCRIPT_WRITE_STRING(udp.address().toString().c_str())
				else
					SCRIPT_WRITE_NIL
			} else if (strcmp(name, "peerAddress") == 0) {
				if(!udp.peerAddress().host().isWildcard())
					SCRIPT_WRITE_STRING(udp.peerAddress().toString().c_str())
				else
					SCRIPT_WRITE_NIL
			}
		}
	SCRIPT_CALLBACK_RETURN
}

int LUAUDPSocket::Set(lua_State* pState) {
	SCRIPT_CALLBACK(LUAUDPSocket,udp)
		lua_rawset(pState,1); // consumes key and value
	SCRIPT_CALLBACK_RETURN
}
