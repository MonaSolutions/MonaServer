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

namespace Events {
	// Can be called by a separated thread!
	struct OnConnection : Event<void(Exception&,const SocketAddress&,SocketFile&)> {};
};

class TCPServer : public virtual Object,
	public Events::OnError,
	public Events::OnConnection {
public:
	TCPServer(const SocketManager& manager);
	virtual ~TCPServer();

	// unsafe-threading
	const SocketAddress&	address() const { return _address; }
	// safe-threading
	SocketAddress&			address(SocketAddress& address) const { std::lock_guard<std::mutex> lock(_mutex);  return address=_address; }

	bool					start(Exception& ex, const SocketAddress& address);
	bool					running() { return _running;  }
	void					stop();

	const SocketManager&	manager() const { return _socket.manager(); }

private:
	Socket::OnReadable::Type	onReadable;

	Socket					_socket;
	mutable std::mutex		_mutex;
	volatile bool			_running;
	SocketAddress			_address;
	
};


} // namespace Mona
