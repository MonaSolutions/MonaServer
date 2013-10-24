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

#include "Mona/TCPServer.h"
#include "Mona/StreamSocket.h"

using namespace std;


namespace Mona {

TCPServer::TCPServer(const SocketManager& manager) : _port(0),ServerSocket(manager) {
}


TCPServer::~TCPServer() {
	stop();
}

bool TCPServer::start(Exception& ex,UInt16 port) {
	if(port==0) {
		ex.set(Exception::SOCKET,"TCPServer port have to be superior to 0");
		return false;
	}
	stop();
	SocketAddress address;
	address.set(IPAddress::Wildcard(),port);
	if (!bind(ex, address) || !listen(ex))
		return false;
	_port=port;
	return true;
}

void TCPServer::stop() {
	close();
	_port=0;
}

void TCPServer::onReadable(Exception& ex) {
	shared_ptr<StreamSocket> pSocket(newClient());
	if(acceptConnection(ex,pSocket))
		clientHandler(pSocket);
}


} // namespace Mona
