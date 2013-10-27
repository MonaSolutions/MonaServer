/* 
	Copyright 2013 Mona - mathieu.poux[a]gmail.com
 
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
#include "Mona/DatagramSocket.h"


namespace Mona {

class UDPSender : public SocketSender, virtual Object {
public:
	UDPSender(bool dump = false) : SocketSender(dump) {}
	UDPSender(const UInt8* data, UInt32 size, bool dump = false) : SocketSender(data, size, dump) {}

	SocketAddress			address;
private:
	UInt32					send(Exception& ex, Socket& socket, const UInt8* data, UInt32 size) {
		return address == SocketAddress::Wildcard() ? ((DatagramSocket&)socket).sendBytes(ex, data, size) : ((DatagramSocket&)socket).sendTo(ex, data, size, address);
	}

	virtual bool			receiver(std::string& address) { address.assign(this->address.toString()); return true; }
};



} // namespace Mona
