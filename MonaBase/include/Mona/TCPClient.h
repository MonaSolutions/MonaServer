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
#include "Mona/StreamSocket.h"
#include <vector>

namespace Mona {


class TCPClient : protected StreamSocket, virtual Object {
public:
	TCPClient(const SocketManager& manager);
	virtual ~TCPClient();

	bool					connect(Exception& ex, const std::string& address);
	bool					connected() { return _connected; }
	bool					send(Exception& ex, const UInt8* data, UInt32 size);

	void					disconnect();

	const std::string&		address(Exception& ex, std::string& address) { SocketAddress temp; return (address = StreamSocket::address(ex, temp).toString()); }
	const std::string&		peerAddress(Exception& ex, std::string& address) { SocketAddress temp; return (address = StreamSocket::peerAddress(ex, temp).toString()); }

private:
	virtual void			onNewData(const UInt8* data,UInt32 size){}
	virtual UInt32			onReception(const UInt8* data,UInt32 size)=0;
	virtual void			onDisconnection(){}

	void					onReadable(Exception& ex);

	int						sendIntern(const UInt8* data,UInt32 size);

	std::vector<UInt8>		_buffer;
	bool					_connected;
};


} // namespace Mona
