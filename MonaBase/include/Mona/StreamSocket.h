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


class StreamSocket : public Socket, virtual Object {
public:
	StreamSocket(const SocketManager& manager) : Socket(manager) {}
	
	bool connect(Exception& ex, const SocketAddress& address) { if (!Socket::connect(ex, address)) return false; setNoDelay(ex, true) /* enabe nodelay per default: OSX really needs that */; return true; }
	
	void shutdown(Exception& ex, ShutdownType type = BOTH) { return Socket::shutdown(ex, type); }

	int receiveBytes(Exception& ex, void* buffer, int length, int flags = 0) { return Socket::receiveBytes(ex, buffer, length, flags); }
	int sendBytes(Exception& ex, const void* buffer, int length, int flags = 0) { return Socket::sendBytes(ex, buffer, length, flags); }

};


} // namespace Mona
