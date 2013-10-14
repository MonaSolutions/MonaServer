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
#include "Mona/Logs.h"
#include "Poco/Net/ServerSocket.h"

using namespace std;
using namespace Poco;
using namespace Poco::Net;

namespace Mona {

TCPServer::TCPServer(const SocketManager& manager) : _port(0),SocketHandler<ServerSocket>(manager) {
}


TCPServer::~TCPServer() {
	stop();
}

bool TCPServer::start(UInt16 port) {
	if(port==0) {
		ERROR("TCPServer port have to be superior to 0");
		return false;
	}
	stop();
	try {
		openSocket(new ServerSocket(port))->setLinger(false,0);
	} catch(Exception& ex) {
		closeSocket();
		ERROR("TCPServer starting error, %s",ex.displayText().c_str())
		return false;
	}
	_port=port;
	return true;
}

void TCPServer::stop() {
	closeSocket();
	_port=0;
}

void TCPServer::onReadable() {
	StreamSocket ss = getSocket()->acceptConnection();
	// enabe nodelay per default: OSX really needs that
	ss.setNoDelay(true);
	clientHandler(ss);
}

void TCPServer::onError(const string& error) {
	ERROR("TCPServer socket, %s",error.c_str())
	UInt16 port = _port;
	stop();
	if(port>0)
		start(port);
}

} // namespace Mona
