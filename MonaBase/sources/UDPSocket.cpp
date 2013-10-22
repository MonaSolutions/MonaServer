/* 
	Copyright 2010 Mona - mathieu.poux[a]gmail.com
 
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

#include "Mona/UDPSocket.h"
#include "Mona/Util.h"
#include "Mona/UDPSender.h"
#include "Poco/Format.h"

using namespace std;
using namespace Poco;
using namespace Poco::Net;

namespace Mona {


UDPSocket::UDPSocket(const SocketManager& manager,bool allowBroadcast) : SocketHandler<DatagramSocket>(manager),_allowBroadcast(allowBroadcast),_bound(false),_connected(false),_buffer(2048) {
	openSocket(new DatagramSocket())->setBroadcast(_allowBroadcast);
}

UDPSocket::~UDPSocket() {
}

void UDPSocket::onReadable() {
	UInt32 available = getSocket()->available();
	if(available==0)
		return;

	if(available>_buffer.size())
		_buffer.resize(available);

	SocketAddress address;
	onReception(&_buffer[0],getSocket()->receiveFrom(&_buffer[0],available,address),address);
}

void UDPSocket::close() {
	if(!_bound && !_connected)
		return;
	_error.clear();
	
	closeSocket();
	openSocket(new DatagramSocket())->setBroadcast(_allowBroadcast);

	_connected = false;
	_bound = false;
}

bool UDPSocket::bind(const Poco::Net::SocketAddress & address) {
	_error.clear();
	if(_bound) {
 		if(Util::SameAddress(getSocket()->address(),address))
			return true;
 		_error = format("UDPSocket already bound on %s, close the socket before",getSocket()->address().toString());
 		return false;
 	}
	if(_connected) {
		_error = "Impossible to bind a connected UDPSocket, close the socket before";
		return false;
	}
	try {
		getSocket()->bind(address,true);
		_bound = true;
	} catch(Poco::Exception& ex) {
		_error = format("Impossible to bind to %s, %s",address.toString(),ex.displayText());
	}
	return _bound;
}

void UDPSocket::connect(const SocketAddress& address) {
	_error.clear();
	_connected = false;
	if(_bound) {
		_error = "Impossible to connect a bound UDPSocket, close the socket before";
		return;
	}
	try {
		getSocket()->connect(address);
		_connected = true;
	} catch(Poco::Exception& ex) {
		_error = format("Impossible to connect to %s, %s",address.toString(),ex.displayText());
	}
}

void UDPSocket::send(const UInt8* data,UInt32 size) {
	_error.clear();
	if(!_connected) {
		_error = "Sending without recipient on a UDPSocket not connected";
		return;
	}
	AutoPtr<UDPSender>(new UDPSender(*this,data,size));
}

void UDPSocket::send(const UInt8* data,UInt32 size,const SocketAddress& address) {
	_error.clear();
	if(size==0)
		return;
	AutoPtr<UDPSender> pSender(new UDPSender(*this,data,size));
	pSender->address = address;
}

} // namespace Mona
