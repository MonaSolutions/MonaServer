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
#include "Mona/SocketManager.h"


using namespace std;


namespace Mona {


UDPSocket::UDPSocket(const SocketManager& manager, bool allowBroadcast) : SocketHandler(manager,Socket::DATAGRAM), _allowBroadcast(allowBroadcast) {

}

void UDPSocket::onReadable(Exception& ex) {
	UInt32 available = socket().available(ex);
	if(ex || available==0)
		return;

	PoolBuffer pBuffer(poolBuffers(),available);
	SocketAddress address;
	int size = socket().receiveFrom(ex,pBuffer->data(), available, address);
	if (ex || size <= 0)
		return;
	pBuffer->resize(size, true);
	onReception(pBuffer,address);
}

bool UDPSocket::send(Exception& ex, const UInt8* data, UInt32 size) {
	if (size == 0)
		return true;
	shared_ptr<UDPSender> pSender(new UDPSender("UDPSender::send",data, size));
	return socket().send(ex, pSender);
}

bool UDPSocket::send(Exception& ex, const UInt8* data, UInt32 size,const SocketAddress& address) {
	if (size == 0)
		return true;
	shared_ptr<UDPSender> pSender(new UDPSender("UDPSender::send",data, size));
	pSender->address.set(address);
	pSender->allowBroadcast = _allowBroadcast;
	return socket().send(ex, pSender);
}

} // namespace Mona
