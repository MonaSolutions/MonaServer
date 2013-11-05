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

TCPClient::TCPClient(const SocketManager& manager) : _connected(false), StreamSocket(manager), _rest(0){
}

TCPClient::TCPClient(const SocketManager& manager, const SocketAddress& peerAddress) : _peerAddress(peerAddress),_connected(true), _rest(0), StreamSocket(manager) {

}

TCPClient::~TCPClient() {
	disconnect();
}

const SocketAddress& TCPClient::address() {
	if (!_address.host().isWildcard())
		return _address;
	Exception ex;
	StreamSocket::address(ex, _address);
	if (ex)
		onError(ex.error());
	return _address;
}

const SocketAddress& TCPClient::peerAddress() {
	if (!_peerAddress.host().isWildcard())
		return _peerAddress;
	Exception ex;
	StreamSocket::peerAddress(ex, _peerAddress);
	if (ex)
		onError(ex.error());
	return _peerAddress;
}


void TCPClient::onReadable(Exception& ex) {
	UInt32 available = StreamSocket::available(ex);
	if (ex)
		return;
	if(available==0) {
		disconnect();
		return;
	}

	if (available > (_buffer.size() - _rest))
		_buffer.resize(available + _rest);

	int received = receiveBytes(ex, &_buffer[_rest], available);
	if (ex)
		return;
	if (received <= 0) {
		disconnect(); // Graceful disconnection
		return;
	}
	onNewData(&_buffer[_rest], received);
	_rest += received;

	while (_rest > 0) {
		UInt32 rest = onReception(&_buffer[0], _rest);
		if (rest > _rest)
			rest = _rest;
		if (rest < _rest) {
			memcpy(&_buffer[0], &_buffer[_rest - rest], rest);
			_rest = rest;
		} else
			break; // nothing to do, waiting new data
	}
}


bool TCPClient::connect(Exception& ex,const SocketAddress& address) {
	disconnect();
	return _connected = StreamSocket::connect(ex, address);
}


void TCPClient::disconnect() {
	if(!_connected)
		return;
	Exception ex;
	shutdown(ex,Socket::RECV);
	close();
	_rest = 0;
	_address.clear();
	_peerAddress.clear();
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
