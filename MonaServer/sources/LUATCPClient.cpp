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

#include "LUATCPClient.h"
#include "LUASocketAddress.h"
#include "Service.h"

using namespace std;
using namespace Mona;


LUATCPClient::LUATCPClient(const SocketAddress& peerAddress,SocketFile& file,const SocketManager& manager, lua_State* pState) : _pState(pState), TCPClient(peerAddress, file, manager),
	_onError([this](const Exception& ex) {_error = ex.error();}),
	_onData([this](PoolBuffer& pBuffer) { return onData(pBuffer);}),
	_onDisconnection([this](TCPClient& client, const SocketAddress& peerAddress) {onDisconnection(peerAddress); }) {

	OnError::subscribe(_onError);
	OnDisconnection::subscribe(_onDisconnection);
	OnData::subscribe(_onData);
}

LUATCPClient::LUATCPClient(const SocketManager& manager,lua_State* pState) : _pState(pState),TCPClient(manager),
	_onError([this](const Exception& ex) {_error = ex.error();}),
	_onData([this](PoolBuffer& pBuffer) { return onData(pBuffer);}),
	_onDisconnection([this](TCPClient& client, const SocketAddress& peerAddress) {onDisconnection(peerAddress); }) {

	OnError::subscribe(_onError);
	OnDisconnection::subscribe(_onDisconnection);
	OnData::subscribe(_onData);
}

LUATCPClient::~LUATCPClient() {
	OnData::unsubscribe(_onData);
	OnDisconnection::unsubscribe(_onDisconnection);
	OnError::unsubscribe(_onError);
}

UInt32 LUATCPClient::onData(PoolBuffer& pBuffer) {
	UInt32 consumed;
	SCRIPT_BEGIN(_pState)
		SCRIPT_MEMBER_FUNCTION_BEGIN(LUATCPClient,*this,"onData")
			SCRIPT_WRITE_BINARY(pBuffer->data(), pBuffer->size())
			SCRIPT_FUNCTION_CALL
			consumed = SCRIPT_READ_UINT(pBuffer->size()); // by default, consume all
		SCRIPT_FUNCTION_END
	SCRIPT_END
	return consumed;
}

void LUATCPClient::onDisconnection(const SocketAddress& peerAddress){
	SCRIPT_BEGIN(_pState)
		SCRIPT_MEMBER_FUNCTION_BEGIN(LUATCPClient,*this,"onDisconnection")
			SCRIPT_WRITE_STRING(peerAddress.toString().c_str())
			if (!_error.empty())
				SCRIPT_WRITE_STRING(_error.c_str())
			SCRIPT_FUNCTION_CALL
		SCRIPT_FUNCTION_END
	SCRIPT_END
	_error.clear();
	Script::ClearObject<LUATCPClient>(_pState, *this);
}

void LUATCPClient::Clear(lua_State* pState, LUATCPClient& client) {
	Script::ClearObject<LUASocketAddress>(pState, client.address());
	Script::ClearObject<LUASocketAddress>(pState, client.peerAddress());
}

int	LUATCPClient::Connect(lua_State* pState) {
	SCRIPT_CALLBACK(LUATCPClient,client)

		Exception ex;
		SocketAddress address(IPAddress::Loopback(),0);
		if (LUASocketAddress::Read(ex, pState, SCRIPT_READ_NEXT(1), address) && client.connect(ex, address)) {
			if (ex)
				SCRIPT_WARN(ex.error())
			SCRIPT_WRITE_BOOL(true)
		} else {
			SCRIPT_ERROR(ex.error())
			SCRIPT_WRITE_BOOL(false)
		}

	SCRIPT_CALLBACK_RETURN
}

int	LUATCPClient::Disconnect(lua_State* pState) {
	SCRIPT_CALLBACK(LUATCPClient,client)
		client.disconnect();
	SCRIPT_CALLBACK_RETURN
}


int	LUATCPClient::Send(lua_State* pState) {
	SCRIPT_CALLBACK(LUATCPClient,client)
		SCRIPT_READ_BINARY(data,size)
		Exception ex;
		client.send(ex,data, size);
		if (ex)
			SCRIPT_WRITE_STRING(ex.error())
	SCRIPT_CALLBACK_RETURN
}

int LUATCPClient::Get(lua_State* pState) {
	SCRIPT_CALLBACK(LUATCPClient,client)
		const char* name = SCRIPT_READ_STRING(NULL);
		if(name) {
			if(strcmp(name,"connect")==0) {
				SCRIPT_WRITE_FUNCTION(LUATCPClient::Connect)
				SCRIPT_FIX_RESULT
			} else if (strcmp(name, "disconnect") == 0) {
				SCRIPT_WRITE_FUNCTION(LUATCPClient::Disconnect)
				SCRIPT_FIX_RESULT
			} else if (strcmp(name, "send") == 0) {
				SCRIPT_WRITE_FUNCTION(LUATCPClient::Send)
				SCRIPT_FIX_RESULT
			} else if (strcmp(name, "address") == 0) {
				Script::AddObject<LUASocketAddress>(pState, client.address());
				SCRIPT_FIX_RESULT
			} else if (strcmp(name, "peerAddress") == 0) {
				Script::AddObject<LUASocketAddress>(pState, client.peerAddress());
				SCRIPT_FIX_RESULT
			} else if (strcmp(name, "idleTime") == 0) {
				SCRIPT_WRITE_NUMBER(client.idleTime())  // change
			} else if (strcmp(name, "connected") == 0)
				SCRIPT_WRITE_BOOL(client.connected())  // change
		}
	SCRIPT_CALLBACK_RETURN
}

int LUATCPClient::Set(lua_State* pState) {
	SCRIPT_CALLBACK(LUATCPClient,client)
		lua_rawset(pState,1); // consumes key and value
	SCRIPT_CALLBACK_RETURN
}
