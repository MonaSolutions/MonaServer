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
#include "Mona/SocketHandler.h"
#include "Poco/Net/DatagramSocket.h"

namespace Mona {

class UDPSocket : private SocketHandler<Poco::Net::DatagramSocket> {
public:
	UDPSocket(const SocketManager& manager,bool allowBroadcast=false);
	virtual ~UDPSocket();

	bool					 bind(const Poco::Net::SocketAddress & address);
	void					 connect(const Poco::Net::SocketAddress& address);
	void					 close();

	void					 send(const Mona::UInt8* data,Mona::UInt32 size);
	void					 send(const Mona::UInt8* data,Mona::UInt32 size,const Poco::Net::SocketAddress& address);

	Poco::Net::SocketAddress address();
	Poco::Net::SocketAddress peerAddress();

	const char*				 error();

private:
	virtual void			onReception(const Mona::UInt8* data,Mona::UInt32 size,const Poco::Net::SocketAddress& address)=0;

	void					onReadable();
	void					onError(const std::string& error);

	std::string					_error;
	std::vector<Mona::UInt8>	_buffer;
	bool						_connected;
	bool						_allowBroadcast;
	bool						_bound;
};

inline Poco::Net::SocketAddress	UDPSocket::address() {
	return (_connected || _bound) ? getSocket()->address() : Poco::Net::SocketAddress();
}

inline Poco::Net::SocketAddress	UDPSocket::peerAddress() {
	return _connected ? getSocket()->peerAddress() : Poco::Net::SocketAddress();
}

inline void UDPSocket::onError(const std::string& error) {
	_error = "UDPSocket error, " + error;
}

inline const char* UDPSocket::error() {
	return _error.empty() ? NULL : _error.c_str();
}

} // namespace Mona
