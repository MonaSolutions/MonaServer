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


UDPSocket::UDPSocket(const SocketManager& manager, bool allowBroadcast) : _broadcasting(false), DatagramSocket(manager), _allowBroadcast(allowBroadcast) {

}


const SocketAddress& UDPSocket::address() {
	if (!_address.host().isWildcard())
		return _address;
	Exception ex;
	DatagramSocket::address(ex, _address);
	if (ex)
		onError(ex.error());
	return _address;
}

const SocketAddress& UDPSocket::peerAddress() {
	if (!_peerAddress.host().isWildcard())
		return _peerAddress;
	Exception ex;
	DatagramSocket::peerAddress(ex, _peerAddress);
	if (ex)
		onError(ex.error());
	return _peerAddress;
}

void UDPSocket::onReadable(Exception& ex) {
	UInt32 available = DatagramSocket::available(ex);
	if(ex || available==0)
		return;

	if (!_pBuffer)
		_pBuffer.reset(new Buffer<UInt8>(available));
	else if (available>_pBuffer->size())
		_pBuffer->resize(available,false);

	SocketAddress address;
	int size = receiveFrom(ex, _pBuffer->data(), available, address);
	if (ex)
		return;
	UInt32 originSize = _pBuffer->size();
	_pBuffer->resize(size);
	onReception(_pBuffer,address);
	if (!_pBuffer.unique())
		_pBuffer.reset(new Buffer<UInt8>(originSize));
	else
		_pBuffer->resetSize();
}

void UDPSocket::close() {
	DatagramSocket::close();
	_broadcasting = false;
	_address.clear();
	_peerAddress.clear();
	_pBuffer.reset();
}

bool UDPSocket::bind(Exception& ex,const SocketAddress& address) {
	bool result = DatagramSocket::bind(ex, address);
	if (result && _allowBroadcast && !_broadcasting) {
		setBroadcast(ex, true);
		_broadcasting = !ex;
	}
	return result;
}

bool UDPSocket::connect(Exception& ex, const SocketAddress& address) {
	bool result = DatagramSocket::connect(ex, address);
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

bool UDPSocket::send(Exception& ex, const UInt8* data, UInt32 size,const SocketAddress& address) {
	if (size == 0)
		return true;
	shared_ptr<UDPSender> pSender(new UDPSender(data, size));
	pSender->address.set(address);
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
