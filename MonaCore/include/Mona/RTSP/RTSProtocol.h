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
#include "Mona/RTSP/RTSPSession.h"

namespace Mona {

class RTSProtocol : public TCProtocol, public virtual Object {
public:
	RTSProtocol(const char* name, Invoker& invoker, Sessions& sessions) : TCProtocol(name, invoker, sessions) {

		setNumber("timeout", 60); // 60s by default

		onConnection = [this](Exception& ex,const SocketAddress& address,SocketFile& file) {
			this->sessions.create<RTSPSession>(address,file,*this,this->invoker); // Create session
		};
		OnConnection::subscribe(onConnection);
	}
	~RTSProtocol() { OnConnection::unsubscribe(onConnection); }
private:
	TCProtocol::OnConnection::Type onConnection;
};


} // namespace Mona
