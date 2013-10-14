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
#include "Poco/Net/StreamSocket.h"

namespace Mona {

class TCPSender : public SocketSender {
public:
	TCPSender(SocketHandler<Poco::Net::StreamSocket>& handler,bool dump=false) : handler(handler),SocketSender(handler,dump),_address(handler.getSocket() ? handler.getSocket()->peerAddress() : Poco::Net::SocketAddress()),_socket(handler.getSocket() ? *handler.getSocket() : Poco::Net::Socket()) {}
	TCPSender(SocketHandler<Poco::Net::StreamSocket>& handler,const Mona::UInt8* data,Mona::UInt32 size,bool dump=false) : handler(handler),SocketSender(handler,data,size,dump),_address(handler.getSocket() ? handler.getSocket()->peerAddress() : Poco::Net::SocketAddress()),_socket(handler.getSocket() ? *handler.getSocket() : Poco::Net::Socket()) {}
	virtual ~TCPSender(){}

	SocketHandler<Poco::Net::StreamSocket>& handler;

private:
	Mona::UInt32					send(const Mona::UInt8* data,Mona::UInt32 size);
	const Poco::Net::SocketAddress&	receiver();

	Poco::Net::StreamSocket			_socket;
	Poco::Net::SocketAddress		_address;
};

inline const Poco::Net::SocketAddress& TCPSender::receiver() {
	return _address;
}

inline Mona::UInt32 TCPSender::send(const Mona::UInt8* data,Mona::UInt32 size) {
	return _socket.sendBytes(data,(int)size);
}


} // namespace Mona
