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
#include "Mona/UDPSender.h"


using namespace std;


namespace Mona {


UDPSocket::UDPSocket(const SocketManager& manager, bool allowBroadcast) : _broadcasting(false), DatagramSocket(manager), _allowBroadcast(allowBroadcast), _buffer(2048) {

}

UDPSocket::~UDPSocket() {
}

void UDPSocket::onReadable(Exception& ex) {
	UInt32 available = DatagramSocket::available(ex);
	if(ex || available==0)
		return;

	if(available>_buffer.size())
		_buffer.resize(available);

	SocketAddress address;
	int size = receiveFrom(ex, &_buffer[0], available, address);
	if (ex)
		return;
	onReception(ex,&_buffer[0], size, address);
}

void UDPSocket::close() {
	DatagramSocket::close();
	_broadcasting = false;
}

bool UDPSocket::bind(Exception& ex,const string& address) {
	SocketAddress temp;
	if (!temp.set(ex, address))
		return false;
	bool result = DatagramSocket::bind(ex, temp);
	if (result && _allowBroadcast && !_broadcasting) {
		setBroadcast(ex, true);
		_broadcasting = !ex;
	}
		
	return result;
}

bool UDPSocket::connect(Exception& ex, const string& address) {
	SocketAddress temp;
	if (!temp.set(ex, address))
		return false;
	bool result = DatagramSocket::connect(ex, temp);
	if (result && _allowBroadcast && !_broadcasting) {
		setBroadcast(ex, true);
		_broadcasting = !ex;
	}
	return result;
}

bool UDPSocket::send(Exception& ex, const UInt8* data, UInt32 size) {
	if (size == 0)
		return true;
	shared_ptr<UDPSender> pSender(new UDPSender(data, size));
	DatagramSocket::send(ex, pSender);
	if (!ex)
		return false;
	if (!ex && _allowBroadcast && !_broadcasting) {
		setBroadcast(ex, true);
		_broadcasting = !ex;
	}
	return true;
}

bool UDPSocket::send(Exception& ex, const UInt8* data, UInt32 size,const string& address) {
	if (size == 0)
		return true;
	shared_ptr<UDPSender> pSender(new UDPSender(data, size));
	if (!pSender->address.set(ex, address))
		return false;
	DatagramSocket::send(ex, pSender);
	if (!ex)
		return false;
	if (!ex && _allowBroadcast && !_broadcasting) {
		setBroadcast(ex, true);
		_broadcasting = !ex;
	}
	return true;
}

} // namespace Mona
