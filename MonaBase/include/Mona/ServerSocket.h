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

#include "Mona/Mona.h"
#include "Mona/Socket.h"


namespace Mona {


class ServerSocket : public Socket, virtual Object {
public:
	ServerSocket(const SocketManager& manager) : Socket(manager) {}

	bool bindWithListen(Exception& ex, const SocketAddress& address, bool reuseAddress = true, int backlog = 64) { return Socket::bindWithListen(ex, address, reuseAddress,backlog); }

	template<typename SocketType, typename ...Args>
	SocketType* acceptConnection(Exception& ex, Args&&... args) { return Socket::acceptConnection<SocketType>(ex, args ...); }
	void		rejectConnection() { Socket::rejectConnection(); }
};


} // namespace Mona
