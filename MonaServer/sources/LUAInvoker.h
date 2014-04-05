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

#include "Mona/Peer.h"
#include "Mona/Invoker.h"
#include "Script.h"

class LUAInvoker {
public:
	static void Init(lua_State *pState, Mona::Invoker& invoker);
	static int Get(lua_State *pState);
	static int Set(lua_State *pState);

	// -1 must be the client table!
	static void AddClient(lua_State *pState, Mona::Invoker& invoker, Mona::Client& client);
	static void RemoveClient(lua_State *pState, Mona::Invoker& invoker,const Mona::Client& client);

	// -1 must be the publication table!
	static void AddPublication(lua_State *pState, Mona::Invoker& invoker, const Mona::Publication& publication);
	static void RemovePublication(lua_State *pState, Mona::Invoker& invoker, const Mona::Publication& publication);

	// -1 must be the group table!
	static void AddGroup(lua_State *pState, Mona::Invoker& invoker, Mona::Group& group);
	static void RemoveGroup(lua_State *pState, Mona::Invoker& invoker, const Mona::Group& group);


private:
	static int  Split(lua_State *pState);
	static int	Md5(lua_State *pState);
	static int	ListFiles(lua_State *pState);
	static int	Sha256(lua_State *pState);

	/// \brief Generic function for serialization
	/// lua -> DataType
	template<typename DataType>
	static int	ToData(lua_State *pState) {
		SCRIPT_CALLBACK(Mona::Invoker,invoker)
		DataType writer(invoker.poolBuffers);
		SCRIPT_READ_DATA(writer)
		SCRIPT_WRITE_BINARY(writer.packet.data(),writer.packet.size())
		SCRIPT_CALLBACK_RETURN
	}

	/// \brief Generic function for serialization
	/// DataType -> lua
	template<typename DataType>
	static int	FromData(lua_State *pState) {
		SCRIPT_CALLBACK(Mona::Invoker,invoker)
		SCRIPT_READ_BINARY(data,size)
		Mona::PacketReader packet(data,size);
		DataType reader(packet);
		SCRIPT_WRITE_DATA(reader,SCRIPT_READ_UINT(0))
		SCRIPT_CALLBACK_RETURN
	}

	static int	ToAMF0(lua_State *pState);
	static int	AddToBlacklist(lua_State *pState);
	static int	RemoveFromBlacklist(lua_State *pState);

	static int  CreateUDPSocket(lua_State *pState);
	static int	CreateTCPServer(lua_State *pState);
	static int	CreateTCPClient(lua_State *pState);
	static int	Publish(lua_State *pState);
	static int	JoinGroup(lua_State *pState);
	static int	AbsolutePath(lua_State *pState);
};
