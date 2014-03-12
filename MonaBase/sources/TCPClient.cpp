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

#include "Mona/TCPClient.h"
#include "Mona/TCPSender.h"
#include "Mona/SocketManager.h"

using namespace std;


namespace Mona {

TCPClient::TCPClient(const SocketManager& manager) :  _pBuffer(manager.poolBuffers),_connected(false), SocketHandler(manager), _rest(0) {
}

TCPClient::TCPClient(const SocketAddress& peerAddress, const SocketManager& manager) :  _pBuffer(manager.poolBuffers), _connected(true), _rest(0), SocketHandler(peerAddress,manager) {

}

TCPClient::~TCPClient() {
	disconnect();
}


void TCPClient::onReadable(Exception& ex) {
	UInt32 available = socket().available(ex);
	if (ex)
		return;

	lock_guard<recursive_mutex> lock(_mutex);

	if(available==0) {
		disconnect();
		return;
	}

	if(available>(_pBuffer->size() - _rest))
		_pBuffer->resize(_rest+available,true);


	int received = socket().receiveBytes(ex,_pBuffer->data()+_rest, available);
	if (ex)
		return;

	if (received <= 0) {
		disconnect(); // Graceful disconnection
		return;
	}
	_rest += received;

	while (_rest > 0) {

		_pBuffer->resize(_rest,true);
		UInt32 rest = onReception(_pBuffer);

		 // To prevent the case where the buffer has been manipulated during onReception call
		if(_pBuffer.empty())
			rest = 0;
		else if (_pBuffer->size() < _rest)
			_rest = _pBuffer->size();

		if (rest > _rest)
			rest = _rest;
		if (rest == 0) {
			// has consumed all
			_pBuffer.release();
			_rest = 0;
			break;
		}
		if (_rest == rest) // no new bytes consumption, wait next reception
			break;


		// has consumed few bytes (but not all)
		// 0 < rest < _rest <= _pBuffer->size()
		memmove(_pBuffer->data(), _pBuffer->data() + (_rest - rest), rest); // move to the beginning

		_rest = rest;
	}

}


bool TCPClient::connect(Exception& ex,const SocketAddress& address) {
	lock_guard<recursive_mutex> lock(_mutex);
	disconnect();
	_connected = socket().connect(ex, address);
	if (!_connected)
		return false;
	SocketHandler::peerAddress(address);
	return true;
}

void TCPClient::disconnect() {
	lock_guard<recursive_mutex> lock(_mutex);
	if(!_connected)
		return;
	Exception ex;
	socket().shutdown(ex);
	close();
	_connected = false;
	_rest = 0;
	_pBuffer.release();
	resetAddresses();
	onDisconnection(); // in last because can call a TCPClient::connect!
}

bool TCPClient::send(Exception& ex,const UInt8* data,UInt32 size) {
	if(size==0)
		return true;
	shared_ptr<TCPSender> pSender(new TCPSender("TCPClient::send",data, size));
	return socket().send(ex, pSender);
}

} // namespace Mona
