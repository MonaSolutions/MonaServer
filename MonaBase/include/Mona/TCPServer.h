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
#include "Mona/Socket.h"


namespace Mona {

class TCPServer : private SocketEvents, virtual Object {
public:
	TCPServer(const SocketManager& manager);
	virtual ~TCPServer();

	// unsafe-threading
	const SocketAddress&	address() { return _address; }
	// safe-threading
	SocketAddress&			address(SocketAddress& address){ std::lock_guard<std::recursive_mutex> lock(_mutex);  return address=_address; }

	bool					start(Exception& ex, const SocketAddress& address);
	bool					running() { return _running;  }
	void					stop();

	const SocketManager&	manager() const { return _socket.manager(); }
protected:
	void close() { stop(); }
private:
	virtual void	onConnection(Exception& ex,const SocketAddress& address,SocketFile& file) = 0;
	// Can be called from one other thread
	void			onReadable(Exception& ex,UInt32 available);

	Socket					_socket;
	std::recursive_mutex	_mutex;
	volatile bool			_running;
	SocketAddress			_address;
	
};


} // namespace Mona
