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
#include "Mona/TCProtocol.h"
#include "Mona/HTTP/HTTPSession.h"

namespace Mona {

class HTTProtocol : public TCProtocol, public virtual Object {
public:
	HTTProtocol(const char* name, Invoker& invoker, Sessions& sessions) : TCProtocol(name, invoker, sessions) {

		setNumber("timeout", 7); // 7 seconds
		setBoolean("index", true); // index directory, if false => forbid directory index, otherwise redirection to index

		// trick for WebSocket Session (use HTTPProtocol file)
		if (!invoker.hasKey("WebSocket.timeout"))
			invoker.setNumber("WebSocket.timeout", 80); // 80 sec, default Websocket timeout (ping is every 40sec)
	
		onConnection = [this](Exception& ex,const SocketAddress& address,SocketFile& file) {
			this->sessions.create<HTTPSession>(address,file,*this,this->invoker); // Create session
		};
		OnConnection::subscribe(onConnection);
	}
	~HTTProtocol() { OnConnection::unsubscribe(onConnection); }
private:
	TCProtocol::OnConnection::Type onConnection;
};


} // namespace Mona
