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
#include "Mona/SocketSender.h"
#include "Mona/Socket.h"

namespace Mona {

class UDPSender : public SocketSender, public virtual Object {
public:
	UDPSender(const char* name,bool dump = false) : SocketSender(name),allowBroadcast(false) {}
	UDPSender(const char* name,const UInt8* data, UInt32 size) : SocketSender(name,data, size),allowBroadcast(false) {}

	SocketAddress			address;
	bool					allowBroadcast;
private:
	UInt32					send(Exception& ex, Socket& socket, const UInt8* data, UInt32 size) {
		return address ? socket.sendTo(ex, data, size, address,allowBroadcast) : socket.sendBytes(ex, data, size);
	}

};



} // namespace Mona
