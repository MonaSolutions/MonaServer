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

#pragma once

#include "Mona/TCPClient.h"
#include "Script.h"

class LUATCPClient : private Mona::TCPClient {
public:
	LUATCPClient(const Mona::SocketAddress& peerAddress,Mona::SocketFile& file,const Mona::SocketManager& manager, lua_State* pState);
	LUATCPClient(const Mona::SocketManager& manager,lua_State* pState);
	~LUATCPClient();

	static int Get(lua_State* pState);
	static int Set(lua_State* pState);

	static void Init(lua_State *pState, LUATCPClient& client) {}
	static void	Clear(lua_State* pState, LUATCPClient& client);
	static void	Delete(lua_State* pState, LUATCPClient& client) { delete &client; }

private:
	Mona::UInt32	onData(Mona::PoolBuffer& pBuffer);
	void			onDisconnection(const Mona::SocketAddress& peerAddress);

	OnError::Type			_onError;
	OnData::Type			_onData;
	OnDisconnection::Type	_onDisconnection;

	static int	Send(lua_State* pState);
	static int  Connect(lua_State* pState);
	static int  Disconnect(lua_State* pState);

	lua_State*			_pState;
	std::string			_error;
};
