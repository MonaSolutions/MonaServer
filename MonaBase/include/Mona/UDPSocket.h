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
#include "Mona/DatagramSocket.h"
#include <vector>

namespace Mona {

class UDPSocket : protected DatagramSocket {
public:
	UDPSocket(const SocketManager& manager,bool allowBroadcast=false);
	virtual ~UDPSocket();

	bool					bind(Exception& ex, const std::string& address);
	bool					connect(Exception& ex, const std::string& address);
	void					close();

	bool					send(Exception& ex, const UInt8* data, UInt32 size);
	bool					send(Exception& ex, const UInt8* data, UInt32 size, const std::string& address);

	const std::string&		address(Exception& ex, std::string& address) { SocketAddress temp; return (address = DatagramSocket::address(ex, temp).toString()); }
	const std::string&		peerAddress(Exception& ex, std::string& address) { SocketAddress temp; return (address = DatagramSocket::peerAddress(ex, temp).toString()); }

private:
	virtual void			onReception(Exception& ex, const UInt8* data, UInt32 size, const SocketAddress& address) = 0;
	void					onReadable(Exception& ex);

	std::vector<UInt8>			_buffer;
	bool						_allowBroadcast;
	bool						_broadcasting;
};



} // namespace Mona
