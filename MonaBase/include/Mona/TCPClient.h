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
#include "Mona/SocketHandler.h"
#include "Poco/Net/StreamSocket.h"

namespace Mona {


class TCPClient : protected SocketHandler<Poco::Net::StreamSocket> {
public:
	TCPClient(const Poco::Net::StreamSocket& socket,const SocketManager& manager);
	TCPClient(const SocketManager& manager);
	virtual ~TCPClient();

	bool					 connect(const Poco::Net::SocketAddress& address);
	bool					 connected();
	void					 disconnect();

	bool					 send(const Mona::UInt8* data,Mona::UInt32 size);

	Poco::Net::SocketAddress address();
	Poco::Net::SocketAddress peerAddress();

	const char*				 error();

private:
	virtual void				onNewData(const Mona::UInt8* data,Mona::UInt32 size){}
	virtual Mona::UInt32		onReception(const Mona::UInt8* data,Mona::UInt32 size)=0;
	virtual void				onDisconnection(){}
	virtual void				onFlush() {}

	void						onReadable();
	void						onError(const std::string& error);

	void						error(const std::string& error);

	int							sendIntern(const Mona::UInt8* data,Mona::UInt32 size);

	std::string					_error;
	std::vector<Mona::UInt8>	_buffer;
};

inline Poco::Net::SocketAddress	TCPClient::address() {
	return getSocket() ? getSocket()->address() : Poco::Net::SocketAddress();
}

inline Poco::Net::SocketAddress	TCPClient::peerAddress() {
	return getSocket() ? getSocket()->peerAddress() : Poco::Net::SocketAddress();
}

inline void TCPClient::onError(const std::string& error) {
	this->error("TCPClient error, " + error);
}

inline bool TCPClient::connected() {
	return getSocket() ? true : false;
}

inline const char* TCPClient::error() {
	return _error.empty() ? NULL : _error.c_str();
}

} // namespace Mona
