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

#include "Script.h"
#include "Mona/Peer.h"

namespace Mona {
	class Invoker;
}

class LUAInvoker {
public:
	static void Init(lua_State *pState, Mona::Invoker& invoker);
	static int Get(lua_State *pState);
	static int Set(lua_State *pState);

	static void AddClient(lua_State *pState, Mona::Invoker& invoker, Mona::Client& client, int indexClient);
	static void RemoveClient(lua_State *pState, Mona::Invoker& invoker,const Mona::Client& client);

	static void AddPublication(lua_State *pState, Mona::Invoker& invoker, const Mona::Publication& publication, int indexPublication);
	static void RemovePublication(lua_State *pState, Mona::Invoker& invoker, const Mona::Publication& publication);

	static void AddGroup(lua_State *pState, Mona::Invoker& invoker, Mona::Group& group, int indexGroup);
	static void RemoveGroup(lua_State *pState, Mona::Invoker& invoker, const Mona::Group& group);


private:
	static int  Split(lua_State *pState);
	static int	Md5(lua_State *pState);
	static int	Sha256(lua_State *pState);
	static int	ToAMF(lua_State *pState);
	static int	ToAMF0(lua_State *pState);
	static int	FromAMF(lua_State *pState);
	static int	AddToBlacklist(lua_State *pState);
	static int	RemoveFromBlacklist(lua_State *pState);

	static int  CreateUDPSocket(lua_State *pState);
	static int	CreateTCPServer(lua_State *pState);
	static int	CreateTCPClient(lua_State *pState);
	static int	Publish(lua_State *pState);
	static int	JoinGroup(lua_State *pState);
	static int	AbsolutePath(lua_State *pState);
};
