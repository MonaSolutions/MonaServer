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

#pragma once

#include "Mona/Mona.h"
#include "Mona/Socket.h"


namespace Mona {


class DatagramSocket : public Socket, virtual Object {
public:
	DatagramSocket(const SocketManager& manager) : Socket(manager, SOCK_DGRAM) {}

	bool bind(Exception& ex, const SocketAddress& address, bool reuseAddress = true) { return Socket::bind(ex, address, reuseAddress); }
	bool connect(Exception& ex, const SocketAddress& address) { return Socket::connect(ex, address); }
	void setBroadcast(Exception& ex, bool flag) { Socket::setBroadcast(ex, flag); }
	bool getBroadcast(Exception& ex) { return Socket::getBroadcast(ex); }

	int receiveBytes(Exception& ex, void* buffer, int length, int flags = 0) { return Socket::receiveBytes(ex, buffer, length, flags); }
	int receiveFrom(Exception& ex, void* buffer, int length, SocketAddress& address, int flags = 0) { return Socket::receiveFrom(ex, buffer, length, address, flags); }

	int sendBytes(Exception& ex, const void* buffer, int length, int flags = 0) { return Socket::sendBytes(ex, buffer, length, flags); }
	int sendTo(Exception& ex, const void* buffer, int length, const SocketAddress& address, int flags = 0) { return Socket::sendTo(ex, buffer, length, address, flags); }
};


} // namespace Mona
