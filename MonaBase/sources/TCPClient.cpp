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

#include "Mona/TCPClient.h"
#include "Mona/TCPSender.h"

using namespace std;


namespace Mona {

TCPClient::TCPClient(const SocketManager& manager) : _connected(false),StreamSocket(manager) {
}

TCPClient::~TCPClient() {
	disconnect();
}

void TCPClient::onReadable(Exception& ex) {
	UInt32 available = StreamSocket::available(ex);
	if (ex)
		return;
	if(available==0) {
		disconnect();
		return;
	}

	UInt32 size = _buffer.size();
	_buffer.resize(size+available);

	int received = receiveBytes(ex,&_buffer[size],available);
	if (ex)
		return;
	if(received<=0) {
		disconnect(); // Graceful disconnection
		return;
	}
	onNewData(&_buffer[size],received);
	available = size+received;

	UInt32 rest = available;
	do {
		available = rest;
		rest = onReception(&_buffer[0],available);
		if(rest>available)
			rest=available;
		if(_buffer.size()>rest) {
			if(available>rest)
				_buffer.erase(_buffer.begin(),_buffer.begin()+(available-rest));
			_buffer.resize(rest);
		}
	} while(rest>0 && rest!=available);
}


bool TCPClient::connect(Exception& ex,const string& address) {
	disconnect();
	SocketAddress temp;
	if (!temp.set(ex, address))
		return false;
	return _connected = StreamSocket::connect(ex, temp);
}

void TCPClient::disconnect() {
	if(!_connected)
		return;
	Exception ex;
	shutdown(ex,Socket::RECV);
	close();
	_buffer.clear();
	onDisconnection();
}

bool TCPClient::send(Exception& ex,const UInt8* data,UInt32 size) {
	if(size==0)
		return true;
	shared_ptr<TCPSender> pSender(new TCPSender(data, size));
	StreamSocket::send(ex, pSender);
	return !ex;
}

} // namespace Mona
