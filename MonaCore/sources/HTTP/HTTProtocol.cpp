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

#include "Mona/HTTP/HTTProtocol.h"
#include "Mona/HTTP/HTTPSession.h"

using namespace Poco;
using namespace Poco::Net;

namespace Mona {

HTTProtocol::HTTProtocol(const char* name,const HTTPParams& params,Gateway& gateway,Invoker& invoker) : TCPServer(invoker.sockets),Protocol(name,invoker,gateway) {
	start(params.port);
}

HTTProtocol::~HTTProtocol() {
	stop();
}

void HTTProtocol::clientHandler(StreamSocket& socket) {
	if(!auth(socket.address()))
		return;
	// Create session!
	gateway.registerSession(new HTTPSession(socket,*this,invoker));
}




} // namespace Mona
