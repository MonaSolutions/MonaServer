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

using namespace std;


namespace Mona {

TCPClient::TCPClient(const SocketManager& manager) : _disconnecting(false),_pBuffer(manager.poolBuffers), _socket(manager),
		onReadable([this](Exception& ex, UInt32 available) {receive(ex,available);}),
		onSending([this](UInt32 size) {lock_guard<mutex> lock(_mutexIdleTime); _idleTime.update(); }) {
	_socket.OnError::subscribe(*this);
	_socket.OnSending::subscribe(onSending);
	_socket.OnReadable::subscribe(onReadable);
}

TCPClient::TCPClient(const SocketAddress& peerAddress, SocketFile& file,const SocketManager& manager) : _disconnecting(false),_peerAddress(peerAddress),_pBuffer(manager.poolBuffers), _socket(file,manager), 
		onReadable([this](Exception& ex, UInt32 available) {receive(ex,available);}),
		onSending([this](UInt32 size) {lock_guard<mutex> lock(_mutexIdleTime); _idleTime.update(); }) {
	_socket.OnError::subscribe(*this);
	_socket.OnReadable::subscribe(onReadable);
	_socket.OnSending::subscribe(onSending);
}

TCPClient::~TCPClient() {
	_socket.OnReadable::unsubscribe(onReadable);
	_socket.OnSending::unsubscribe(onSending);
	_socket.OnError::unsubscribe(*this);
	close();
}

// private method, call by onReadable
void TCPClient::close() {
	if (!_socket.connected())
		return;
	if (!_disconnecting) {
		Exception ex;
		_socket.flush(ex); // try to flush what is possible before disconnection!
		_socket.shutdown(ex);
	}
	_disconnecting = false;
	_socket.close();
	_pBuffer.release();
	SocketAddress peerAddress;
	{
		lock_guard<mutex> lock(_mutex);
		_address.clear();
		peerAddress.set(_peerAddress);
		_peerAddress.clear();
	}
	OnDisconnection::raise(*this,peerAddress); // in last because can delete this
}


bool TCPClient::connect(Exception& ex,const SocketAddress& address) {
	lock_guard<mutex> lock(_mutex);
	if (!_socket.connect(ex, address))
		return false;
	_peerAddress.set(address);
	return true;
}

void TCPClient::receive(Exception& ex, UInt32 available) {
	
	if (available>0)
		onSending(0); // to update _idleTime

	if (available == 0) {
		close(); // Graceful disconnection
		return;
	}

	UInt32 size(_pBuffer->size());
	_pBuffer->resize(size+available,true);
	
	Exception exRecv;
	
	int received = _socket.receiveBytes(exRecv,_pBuffer->data()+size, available);

	if (received <= 0) {
		if (exRecv)
			OnError::raise(exRecv); // to be before onDisconnection!
		close(); // Graceful disconnection
		return;
	} else if (exRecv)
		ex = exRecv; // received > 0, so WARN

	_pBuffer->resize(size+received,true);
	size = 0;

	while(size<_pBuffer->size()) { // while everything is not consumed

		if (size)
			_pBuffer->clip(size);
		
		size = OnData::raise<0xFFFFFFFF>(_pBuffer); // consumed

		if (!size) // if no consumption
			return; // no release here
	}

	_pBuffer.release();
}

void TCPClient::disconnect() {
	lock_guard<mutex> lock(_mutex);
	if (_disconnecting || !_socket.connected())
		return;
	_disconnecting = true;
	Exception ex;
	_socket.flush(ex); // try to flush what is possible before disconnection!
	_socket.shutdown(ex,Socket::SEND);
}

bool TCPClient::send(Exception& ex,const UInt8* data,UInt32 size) {
	if (size == 0)
		return true;
	shared_ptr<TCPSender> pSender(new TCPSender("TCPClient::send",data, size));
	return _socket.send(ex, pSender);
}

} // namespace Mona
