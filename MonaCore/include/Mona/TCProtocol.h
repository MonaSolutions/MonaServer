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
#include "Mona/Protocol.h"
#include "Mona/TCPServer.h"
#include "Mona/Logs.h"

namespace Mona {


class TCProtocol : public Protocol , public virtual Object,
	public Events::OnConnection,
	public Events::OnError {
public:

	bool load(Exception& ex, const SocketAddress& address) {
		if (!hasKey("timeout"))
			WARN("Protocol ", name, " has no TCP connection timeout");
		return _server.start(ex, address);
	}

protected:
	TCProtocol(const char* name, Invoker& invoker, Sessions& sessions) : _server(invoker.sockets), Protocol(name, invoker, sessions) {
		if (!OnError::subscribed()) {
			onError = [this](const Exception& ex) { WARN("Protocol ", this->name, ", ", ex.error()); };
			_server.OnError::subscribe(onError);
		} else
			_server.OnError::subscribe(*this);

		onConnection = [this](Exception& ex,const SocketAddress& address,SocketFile& file) {
			if(!auth(address))
				return;
			OnConnection::raise(ex,address,file);
		};

		_server.OnConnection::subscribe(onConnection);
	}
	virtual ~TCProtocol() {
		if (onError)
			_server.OnError::unsubscribe(onError);
		else
			_server.OnError::unsubscribe(*this);
		_server.OnConnection::unsubscribe(onConnection);
	}

private:
	TCPServer::OnError::Type	  onError;
	TCPServer::OnConnection::Type onConnection;

	TCPServer _server;
};


} // namespace Mona
