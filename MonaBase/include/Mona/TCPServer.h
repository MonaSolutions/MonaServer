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


#include "Mona/SocketManager.h"
#include "Poco/Net/ServerSocket.h"

namespace Mona {

class TCPServer : protected SocketHandler<Poco::Net::ServerSocket> {
public:
	TCPServer(const SocketManager& manager);
	virtual ~TCPServer();

	bool			start(Mona::UInt16 port);
	bool			running();
	Mona::UInt16	port();
	void			stop();

private:
	virtual void	clientHandler(Poco::Net::StreamSocket& socket)=0;


	void	onReadable();
	void	onError(const std::string& error);

	Mona::UInt16				_port;
};

inline bool	TCPServer::running() {
	return _port>0;
}

inline Mona::UInt16	TCPServer::port() {
	return _port;
}

} // namespace Mona
