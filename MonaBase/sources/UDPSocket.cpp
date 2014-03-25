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


#include "Mona/UDPSocket.h"
#include "Mona/UDPSender.h"


using namespace std;


namespace Mona {


UDPSocket::UDPSocket(const SocketManager& manager, bool allowBroadcast) : _socket(manager,Socket::DATAGRAM), _allowBroadcast(allowBroadcast) {

	onReadable = [this](Exception& ex,UInt32 available) {
		if(available==0)
			return;

		PoolBuffer pBuffer(_socket.manager().poolBuffers,available);
		SocketAddress address;
		int size = _socket.receiveFrom(ex,pBuffer->data(), available, address);
		if (ex || size <= 0)
			return;
		pBuffer->resize(size, true);

		OnPacket::raise(pBuffer,address);
	};
	_socket.OnError::subscribe(*this);
	_socket.OnReadable::subscribe(onReadable);
}

UDPSocket::~UDPSocket() {
	_socket.OnReadable::unsubscribe(onReadable);
	_socket.OnError::unsubscribe(*this);
	close(); // to try to flush in same time!
}


bool UDPSocket::connect(Exception& ex, const SocketAddress& address) {
	bool result = _socket.connect(ex, address, _allowBroadcast); 
	resetAddresses();
	return result;
}

void UDPSocket::disconnect() {
	Exception ex;
	_socket.flush(ex); // try to flush the remaing packet
	_socket.connect(ex, SocketAddress::Wildcard());
	resetAddresses();
}

bool UDPSocket::bind(Exception& ex, const SocketAddress& address) {
	bool result = _socket.bind(ex, address);
	resetAddresses();
	return result;
}

void UDPSocket::close() {
	Exception ex;
	_socket.flush(ex); // try to flush the remaing packet
	_socket.close();
	resetAddresses();
}

bool UDPSocket::send(Exception& ex, const UInt8* data, UInt32 size) {
	if (size == 0)
		return true;
	shared_ptr<UDPSender> pSender(new UDPSender("UDPSender::send",data, size));
	return _socket.send(ex, pSender);
}

bool UDPSocket::send(Exception& ex, const UInt8* data, UInt32 size,const SocketAddress& address) {
	if (size == 0)
		return true;
	shared_ptr<UDPSender> pSender(new UDPSender("UDPSender::send",data, size));
	pSender->address.set(address);
	pSender->allowBroadcast = _allowBroadcast;
	return _socket.send(ex, pSender);
}

} // namespace Mona
