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
#include "Mona/Logs.h"
#include "Poco/Format.h"

using namespace std;
using namespace Poco;
using namespace Poco::Net;

namespace Mona {

TCPClient::TCPClient(const StreamSocket& socket,const SocketManager& manager) : SocketHandler<StreamSocket>(manager) {
	openSocket(new StreamSocket(socket));
}

TCPClient::TCPClient(const SocketManager& manager) : SocketHandler<StreamSocket>(manager) {
}

TCPClient::~TCPClient() {
	disconnect();
}

void TCPClient::error(const string& error) {
	_error = error;
	disconnect();
}

void TCPClient::onReadable() {
	UInt32 available = getSocket()->available();
	if(available==0) {
		disconnect();
		return;
	}

	UInt32 size = _buffer.size();
	_buffer.resize(size+available);

	int received = getSocket()->receiveBytes(&_buffer[size],available);
	if(received<=0) {
		disconnect(); // Graceful disconnection
		return;
	}
	onNewData(&_buffer[size],received);
	available = size+received;

	UInt32 rest = available;
	bool consumed = false;
	do {
		available = rest;
		rest = onReception(&_buffer[0],available);
		if(rest>available) {
			WARN("onReception has returned a 'rest' value more important than the available value (%u>%u)",rest,available);
			rest=available;
		} else if(rest<available)
			consumed=true;
		if(_buffer.size()>rest) {
			if(available>rest)
				_buffer.erase(_buffer.begin(),_buffer.begin()+(available-rest));
			_buffer.resize(rest);
		}
	} while(rest>0 && rest!=available);
	if(consumed)
		onFlush();
}


bool TCPClient::connect(const SocketAddress& address) {
	disconnect();
	_error.clear();
	try {
		openSocket(new StreamSocket(address))->setNoDelay(true); // enabe nodelay per default: OSX really needs that
	} catch(Exception& ex) {
		closeSocket();
		error(format("Impossible to connect to %s, %s",address.toString(),ex.displayText()));
	}
	return connected();
}

void TCPClient::disconnect() {
	if(!getSocket())
		return;
	try {getSocket()->shutdownReceive();} catch(...){}
	closeSocket();
	_buffer.clear();
	onDisconnection();
}

bool TCPClient::send(const UInt8* data,UInt32 size) {
	if(!getSocket()) {
		if(!error())
			error("TCPClient not connected");
		return false;
	}
	if(size==0)
		return true;
	AutoPtr<TCPSender>(new TCPSender(*this,data,size));
	return true;
}

} // namespace Mona
